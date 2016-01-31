/* 
 * File:   StreamServer.cpp
 * Author: Alexandre Brandão Lustosa
 * 
 * Created on 4 de Março de 2014, 09:34
 */

/*
 *      O que acontce ser houver error no sevidor, gerar SIGTERM?
 *      Quais são as possíveis falhas do accept?
 */

#include "StreamServer.h"

bool StreamServer::running = false;

void StreamServer::initService(int Port, int BufferSize,
        int MaxListenConnections,  bool DebugIsOn) {
    struct sockaddr_in serv_addr;
    int optval = 1;
    socklen_t optlen = sizeof (optval);

    this->port = Port;
    this->bufferSize = BufferSize;
    this->maxListenConnections = MaxListenConnections;
    this->maxFd = 0;
    this->running = true;
    this->debugIsOn = DebugIsOn;

    if (debugIsOn) printf("StreamServer Initializade. Port: %.4d\n", this->port);

    server = socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(this->port);

    if (server < 0) {
        if (debugIsOn) printf("StreamServer error opening server socket. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        if (debugIsOn) printf("StreamServer error on set keep alive. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        shutdown(server, SHUT_RDWR);
        close(server);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
        if (debugIsOn) printf("StreamServer error on set reuse address. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        shutdown(server, SHUT_RDWR);
        close(server);
        exit(EXIT_FAILURE);
    }

    if (fcntl(server, F_SETFL, O_NONBLOCK) < 0) {
        if (debugIsOn) printf("StreamServer error on set non block option. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        shutdown(server, SHUT_RDWR);
        close(server);
        exit(EXIT_FAILURE);
    }

    if (fcntl(server, F_SETFL, O_ASYNC) < 0) {
        if (debugIsOn) printf("StreamServer error on set async option. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        shutdown(server, SHUT_RDWR);
        close(server);
        exit(EXIT_FAILURE);
    }

    if (bind(server, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        if (debugIsOn) printf("StreamServer error on bind. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        shutdown(server, SHUT_RDWR);
        close(server);
        exit(EXIT_FAILURE);
    }

    if (listen(server, this->maxListenConnections) < 0) {
        if (debugIsOn) printf("StreamServer error on listen. Port: %.4d\n", this->port);
        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
        shutdown(server, SHUT_RDWR);
        close(server);
        exit(EXIT_FAILURE);
    }
    if (debugIsOn) printf("StreamServer on listen. Port: %.4d\n", this->port);

    this->maxFd = this->server;
    FD_ZERO(&this->masterSet);
    FD_SET(this->server, &this->masterSet);

    signal(SIGPIPE, SIG_IGN);

    int pthread_res = pthread_create(&thAcceptClients, NULL, &StreamServer::AccpetClients, this);
    if (pthread_res == 0) {
        pthread_detach(thAcceptClients);
        if (debugIsOn) printf("StreamServer Created. Port: %.4d\n", this->port);
    } else {
        if (debugIsOn) printf("StreamServer error thread accept clients. Port: %.4d, Error: %.4d\n", this->port, pthread_res);
    }    
}

void* StreamServer::AccpetClients(void* arg) {
    StreamServer *self = (StreamServer*)arg;

    self->AccpetClientsEvent();
}

void StreamServer::AccpetClientsEvent() {

    fd_set acceptSet;
    char buffer[this->bufferSize];

    memset(buffer, 0, this->bufferSize);

    if (debugIsOn) printf("StreamServer on AccpetClients. Port: %.4d\n", this->port);

    this->acceptRunning = true;

    while (this->running == true) {
        memcpy(&acceptSet, &this->masterSet, sizeof (this->masterSet));

        if (debugIsOn) printf("StreamServer wait on select. Port: %.4d\n", this->port);

        int selectFdCount = 0;

        selectFdCount = select(this->maxFd + 1, &acceptSet, NULL, NULL, NULL);

        if (selectFdCount < 0) {
            if (errno == EWOULDBLOCK) {
                if (debugIsOn) printf("StreamServer EWOULDBLOCK on select. Port: %.4d\n", this->port);
                if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                continue;
            } else {
                if (errno == ENOTCONN) {
                    if (debugIsOn) printf("StreamServer error on select, try again. Port: %.4d\n", this->port);
                    if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                    continue;
                } else {
                    if (debugIsOn) printf("StreamServer error on select, server will close. Port: %.4d\n", this->port);
                    if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                    break;
                }
            }
        }

        if (selectFdCount == 0) {
            if (debugIsOn) printf("StreamServer timeout on select. Port: %.4d\n", this->port);
            if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
            break;
        }

        if (debugIsOn) printf("StreamServer selected succesfully. Port: %.4d, Select Result: %.4d\n", this->port, selectFdCount);

        for (int index = 0; index <= this->maxFd && selectFdCount > 0; index++) {
            if (FD_ISSET(index, &acceptSet)) {
                selectFdCount -= 1;

                if (index == this->server) {
                    if (debugIsOn) printf("StreamServer ready for accept incoming connections. Port: %.4d\n", this->port);
                    int newClient = accept(this->server, NULL, NULL);
                    if (newClient < 0) {
                        if (errno != EWOULDBLOCK) {
                            if (debugIsOn) printf("StreamServer error on accept. Port: %.4d\n", this->port);
                            if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                            this->running = false;
                            break;
                        }
                    } else {                        
                        FD_SET(newClient, &this->masterSet);
                        if (newClient > this->maxFd) this->maxFd = newClient;
                        if (debugIsOn) printf("StreamServer client accepted. Port: %.4d, Client: %.4d\n", this->port, newClient);
                    }
                    if (debugIsOn) printf("StreamServer end for accept incoming connections. Port: %.4d\n", this->port);
                } else {
                    if (this->running == true) {
                        int size = recv(index, buffer, this->bufferSize, 0);

                        if (size < 0) {
                            if (errno != EWOULDBLOCK) {
                                shutdown(index, SHUT_RDWR);
                                close(index);
                                FD_CLR(index, &this->masterSet);
                                if (debugIsOn) printf("StreamServer Read error, client disconnected. Port: %.4d, Client: %.4d\n", this->port, index);
                                if ((errno != 0) && (debugIsOn)) {
                                    printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                                }
                            }
                        } else {
                            if (size == 0) {
                                shutdown(index, SHUT_RDWR);
                                close(index);
                                FD_CLR(index, &this->masterSet);
                                if (debugIsOn) printf("StreamServer Read client disconected. Port: %.4d, Client: %.4d\n", this->port, index);
                                if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                            } else {
                                if (debugIsOn) printf("StreamServer received %.4d bytes. Port: %.4d, Client: %.4d\n", size, this->port, index);
#ifdef ECHO_TEST                                
                                this->Write(buffer,  size);
#else
#ifdef ECHO_TEST_ONLYONE
                                this->Write(index, buffer,  size);
#endif                                
#endif                                
                                this->ReceivedData(buffer, size);
                            }
                        }
                    }
                }
            }
        }
    }

    if (debugIsOn) printf("StreamServer Clossing All Connections. Port: %.4d\n", this->port);
    for (int index = 0; index <= this->maxFd; ++index) {
        if (FD_ISSET(index, &this->masterSet)) {
            shutdown(index, SHUT_RDWR);
            close(index);
            FD_CLR(index, &this->masterSet);
        }
    }

    if (debugIsOn) printf("StreamServer Clossing Server. Port: %.4d\n", this->port);
    FD_CLR(this->server, &this->masterSet);
    shutdown(this->server, SHUT_RDWR);
    close(this->server);

    this->acceptRunning = false;

    if (debugIsOn) printf("StreamServer AcceptClients End. Port: %.4d\n", this->port);

    exit(EXIT_FAILURE);
}

int StreamServer::Write(int fd, char *buffer, int size){
    if (debugIsOn) printf("StreamServer Write Init. Port: %.4d\n", this->port);

    int written = 0;

    if (this->running) {

        if (fd != this->server) {
            if (FD_ISSET(fd, &this->masterSet)) {
                int written = send(fd, buffer, size, MSG_NOSIGNAL);
                if (written > -1) {
                    if (debugIsOn) printf("StreamServer Written %.4d bytes. Port: %.4d, Client: %.4d\n", written, this->port, fd);
                } else {
                    if (errno != EWOULDBLOCK) {
                        shutdown(fd, SHUT_RDWR);
                        close(fd);
                        FD_CLR(fd, &this->masterSet);
                        if (debugIsOn) printf("StreamServer Write client desconected. Port: %.4d, Client: %.4d\n", this->port, fd);
                        if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                    } else {
                        if (debugIsOn) printf("StreamServer Write EWOULDBLOCK, will try again. Port: %.4d, Client: %.4d\n", this->port, fd);
                    }
                }
            }            
        }
    } else {
        if (debugIsOn) printf("StreamServer Write, server not running. Port: %.4d\n", this->port);
    }
    if (debugIsOn) printf("StreamServer Write End. Port: %.4d\n", this->port);
    return written;
}

int StreamServer::Write(char *buffer, int size) {
    if (debugIsOn) printf("StreamServer Write Init. Port: %.4d\n", this->port);

    int written = 0;

    if (this->running) {

        for (int index = 0; index <= this->maxFd; ++index) {
            if (index != this->server) {
                if (FD_ISSET(index, &this->masterSet)) {
                    int written = send(index, buffer, size, MSG_NOSIGNAL);
                    if (written > -1) {
                        if (debugIsOn) printf("StreamServer Written %.4d bytes. Port: %.4d, Client: %.4d\n", written, this->port, index);
                    } else {
                        if (errno != EWOULDBLOCK) {
                            shutdown(index, SHUT_RDWR);
                            close(index);
                            FD_CLR(index, &this->masterSet);
                            if (debugIsOn) printf("StreamServer Write client desconected. Port: %.4d, Client: %.4d\n", this->port, index);
                            if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                        } else {
                            if (debugIsOn) printf("StreamServer Write EWOULDBLOCK, will try again. Port: %.4d, Client: %.4d\n", this->port, index);
                        }
                    }
                }
            }
        }
    } else {
        if (debugIsOn) printf("StreamServer Write, server not running. Port: %.4d\n", this->port);
    }
    if (debugIsOn) printf("StreamServer Write End. Port: %.4d\n", this->port);
    return written;
}

StreamServer::~StreamServer() {
    StreamServer::running = false;

    if (debugIsOn) printf("StreamServer Closing All Connections. Port: %.4d\n", this->port);
    for (int index = 0; index <= this->maxFd; ++index) {
        if (FD_ISSET(index, &this->masterSet)) {
            shutdown(index, SHUT_RDWR);
            close(index);
            FD_CLR(index, &this->masterSet);
        }
    }

    if (debugIsOn) printf("StreamServer Closing Server. Port: %.4d\n", this->port);
    if (FD_ISSET(this->server, &this->masterSet)) {
        shutdown(this->server, SHUT_RDWR);
        close(this->server);
        FD_CLR(this->server, &this->masterSet);
    }

    if (debugIsOn) printf("StreamServer Closing threads. Port: %.4d\n", this->port);

    pthread_cancel(this->thAcceptClients);

    if (debugIsOn) printf("StreamServer Destroy. Port: %.4d\n", this->port);
}