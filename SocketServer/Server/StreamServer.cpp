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

template<class T>
bool StreamServer<T>::running = false;

template<class T>
StreamServer<T>::StreamServer(T *instance, int Port, int BufferSize,
        int MaxListenConnections, int MaxClients, bool useHeartBeat,
        bool DebugIsOn) {
    struct sockaddr_in serv_addr;
    int optval = 1;
    socklen_t optlen = sizeof (optval);

    this->selfClass = instance;
    this->port = Port;
    this->bufferSize = BufferSize;
    this->maxListenConnections = MaxListenConnections;
    this->maxClients = MaxClients;
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

    int tryAgain = 0;
    do {
        int pthread_res = pthread_create(&thAcceptClients, NULL, &StreamServer<T>::AccpetClients, this);
        if (pthread_res == 0) {
            pthread_detach(thAcceptClients);
            break;
        } else {
            if (pthread_res == EAGAIN) {
                tryAgain++;
            } else {
                if (debugIsOn) printf("StreamServer error thread accept clients. Port: %.4d, Error: %.4d\n", this->port, pthread_res);
                exit(EXIT_FAILURE);
            }
        }
    } while (tryAgain < 3);

    if (tryAgain >= 3) {
        if (debugIsOn) printf("StreamServer error try again thread accept three times. Port: %.4d\n", this->port);
        exit(EXIT_FAILURE);
    }

    if (useHeartBeat) {
        tryAgain = 0;
        do {
            int pthread_res = pthread_create(&thHeartBeat, NULL, &StreamServer<T>::HeartBeat, this);
            if (pthread_res == 0) {
                pthread_detach(thHeartBeat);
                break;
            } else {
                if (pthread_res == EAGAIN) {
                    tryAgain++;
                } else {
                    if (debugIsOn) printf("StreamServer error thread heartbeat clients. Port: %.4d, Error: %.4d\n", this->port, pthread_res);
                    exit(EXIT_FAILURE);
                }
            }

        } while (tryAgain < 3);
    }

    if (tryAgain >= 3) {
        if (debugIsOn) printf("StreamServer error try again thread heartbeat three times. Port: %.4d\n", this->port);
        exit(EXIT_FAILURE);
    }

    if (debugIsOn) printf("StreamServer Created. Port: %.4d\n", this->port);
}

template<class T>
void* StreamServer<T>::AccpetClients(void* arg) {
    StreamServer<T> *self = (StreamServer<T>*)arg;

    self->AccpetClientsEvent(arg);
}

template<class T>
void StreamServer<T>::AccpetClientsEvent(void* arg) {
    StreamServer<T> *self = (StreamServer<T>*)arg;

    fd_set acceptSet;
    char buffer[self->bufferSize];

    memset(buffer, 0, self->bufferSize);

    if (debugIsOn) printf("StreamServer on AccpetClients. Port: %.4d\n", self->port);

    self->acceptRunning = true;

    while (self->running == true) {
        memcpy(&acceptSet, &self->masterSet, sizeof (self->masterSet));

        if (debugIsOn) printf("StreamServer wait on select. Port: %.4d\n", self->port);

        int selectFdCount = 0;

        selectFdCount = select(self->maxFd + 1, &acceptSet, NULL, NULL, NULL);

        if (selectFdCount < 0) {
            if (errno == EWOULDBLOCK) {
                if (debugIsOn) printf("StreamServer EWOULDBLOCK on select. Port: %.4d\n", self->port);
                if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                continue;
            } else {
                if (errno == ENOTCONN) {
                    if (debugIsOn) printf("StreamServer error on select, try again. Port: %.4d\n", self->port);
                    if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                    continue;
                } else {
                    if (debugIsOn) printf("StreamServer error on select, server will close. Port: %.4d\n", self->port);
                    if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                    break;
                }
            }
        }

        if (selectFdCount == 0) {
            if (debugIsOn) printf("StreamServer timeout on select. Port: %.4d\n", self->port);
            if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
            break;
        }

        if (debugIsOn) printf("StreamServer selected succesfully. Port: %.4d, Select Result: %.4d\n", self->port, selectFdCount);

        for (int index = 0; index <= self->maxFd && selectFdCount > 0; index++) {
            if (FD_ISSET(index, &acceptSet)) {
                selectFdCount -= 1;

                if (index == self->server) {
                    if (debugIsOn) printf("StreamServer ready for accept incoming connections. Port: %.4d\n", self->port);
                    int newClient = accept(self->server, NULL, NULL);
                    if (newClient < 0) {
                        if (errno != EWOULDBLOCK) {
                            if (debugIsOn) printf("StreamServer error on accept. Port: %.4d\n", self->port);
                            if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                            self->running = false;
                            break;
                        }
                    } else {
                        int totalClientsConnected = this->GetTotalConnectedClients();
                        if (totalClientsConnected < this->maxClients) {
                            FD_SET(newClient, &self->masterSet);
                            if (newClient > self->maxFd) self->maxFd = newClient;
                            if (debugIsOn) printf("StreamServer client accepted. Port: %.4d, Client: %.4d\n", self->port, newClient);
                            if (debugIsOn) printf("StreamServer total connections: %.4d\n", totalClientsConnected);
                        } else {
                            shutdown(newClient, SHUT_RDWR);
                            close(newClient);
                            if (debugIsOn) printf("StreamServer cliente not accepted, connection limit error. Port: %.4d, Client: %.4d\n", self->port, newClient);
                            if (debugIsOn) printf("StreamServer total connections: %.4d\n", totalClientsConnected);
                        }
                    }
                    if (debugIsOn) printf("StreamServer end for accept incoming connections. Port: %.4d\n", self->port);
                } else {
                    if (self->running == true) {
                        int size = recv(index, buffer, self->bufferSize, 0);

                        if (size < 0) {
                            if (errno != EWOULDBLOCK) {
                                shutdown(index, SHUT_RDWR);
                                close(index);
                                FD_CLR(index, &self->masterSet);
                                if (debugIsOn) printf("StreamServer Read error, client disconnected. Port: %.4d, Client: %.4d\n", self->port, index);
                                if ((errno != 0) && (debugIsOn)) {
                                    printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                                }
                            }
                        } else {
                            if (size == 0) {
                                shutdown(index, SHUT_RDWR);
                                close(index);
                                FD_CLR(index, &self->masterSet);
                                if (debugIsOn) printf("StreamServer Read client disconected. Port: %.4d, Client: %.4d\n", self->port, index);
                                if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                            } else {
                                if (debugIsOn) printf("StreamServer received %.4d bytes. Port: %.4d, Client: %.4d\n", size, self->port, index);
#ifdef ECHO_TEST                                
                                self->Write(buffer,  size);
#endif                                
                                if (self->ptr != NULL) {
                                    (selfClass->*ptr)(buffer,  size);
                                    if (debugIsOn) printf("StreamServer OnReceiveNewData Event has sent %.4d bytes. Port: %.4d, Client: %.4d\n", size, self->port, index);
                                } else {
                                    if (debugIsOn) printf("StreamServer OnReceiveNewData Event is NULL. Port: %.4d, Client: %.4d\n", self->port, index);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (debugIsOn) printf("StreamServer Clossing All Connections. Port: %.4d\n", self->port);
    for (int index = 0; index <= self->maxFd; ++index) {
        if (FD_ISSET(index, &self->masterSet)) {
            shutdown(index, SHUT_RDWR);
            close(index);
            FD_CLR(index, &self->masterSet);
        }
    }

    if (debugIsOn) printf("StreamServer Clossing Server. Port: %.4d\n", self->port);
    FD_CLR(self->server, &self->masterSet);
    shutdown(self->server, SHUT_RDWR);
    close(self->server);

    self->acceptRunning = false;

    if (debugIsOn) printf("StreamServer AcceptClients End. Port: %.4d\n", self->port);

    exit(EXIT_FAILURE);
}

template<class T>
int StreamServer<T>::Write(char *buffer, int size) {
    if (debugIsOn) printf("StreamServer Write Init. Port: %.4d\n", this->port);

    int written = 0;
    int tryAgain = 1;

    if (this->running) {

        this->writeRunning = true;

        for (int index = 0; index <= this->maxFd; ++index) {
            if (index != this->server) {
                if (FD_ISSET(index, &this->masterSet)) {
                    while ((tryAgain > 0) && (tryAgain <= 3)) {
                        int written = send(index, buffer, size, MSG_NOSIGNAL);
                        if (written > -1) {
                            tryAgain = 0;
                            if (debugIsOn) printf("StreamServer Written %.4d bytes. Port: %.4d, Client: %.4d\n", written, this->port, index);
                        } else {
                            if (errno != EWOULDBLOCK) {
                                shutdown(index, SHUT_RDWR);
                                close(index);
                                FD_CLR(index, &this->masterSet);
                                tryAgain = 0;
                                if (debugIsOn) printf("StreamServer Write client desconected. Port: %.4d, Client: %.4d\n", this->port, index);
                                if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", this->port, errno, strerror(errno));
                            } else {
                                if (debugIsOn) printf("StreamServer Write EWOULDBLOCK, will try again. Port: %.4d, Client: %.4d\n", this->port, index);
                                tryAgain++;
                            }
                        }
                    }
                    tryAgain = 1;
                }
            }
        }
        this->writeRunning = false;
    } else {
        if (debugIsOn) printf("StreamServer Write, server not running. Port: %.4d\n", this->port);
    }
    if (debugIsOn) printf("StreamServer Write End. Port: %.4d\n", this->port);
    return written;
}

template<class T>
void* StreamServer<T>::HeartBeat(void* arg) {
    StreamServer *self = (StreamServer*) arg;

    self->HeartBeatEvent(arg);
}

template<class T>
void StreamServer<T>::HeartBeatEvent(void* arg) {
    StreamServer *self = (StreamServer*) arg;

    if (debugIsOn) printf("StreamServer Write Init. Port: %.4d\n", self->port);

    int size = 4;
    char buffer[4] = {'H', 'B', 'H', 'B'};
    int tryAgain = 1;
    int totalClients;

    self->hearbeatRunning = true;

    sleep(HEARTBEAT_INTERVAL);

    while (self->running) {
        totalClients = 0;
        for (int index = 0; index <= self->maxFd; ++index) {
            if (index != self->server) {
                if (FD_ISSET(index, &self->masterSet)) {
                    while ((tryAgain > 0) && (tryAgain <= 3)) {
                        int sendRes = send(index, buffer, size, MSG_NOSIGNAL);
                        if (sendRes > -1) {
                            tryAgain = 0;
                            if (debugIsOn) printf("StreamServer HeartBeat sent %.4d bytes. Port: %.4d, Client: %.4d\n", size, self->port, index);
                            totalClients++;
                        } else {
                            if (errno != EWOULDBLOCK) {
                                shutdown(index, SHUT_RDWR);
                                close(index);
                                FD_CLR(index, &self->masterSet);
                                tryAgain = 0;
                                if (debugIsOn) printf("StreamServer HeartBeat client desconected. Port: %.4d, Client: %.4d\n", self->port, index);
                                if (debugIsOn) printf("Socket Port %.4d. Error: %.4d - %s\n", self->port, errno, strerror(errno));
                            } else {
                                if (debugIsOn) printf("StreamServer HeartBeat EWOULDBLOCK, will try again. Port: %.4d, Client: %.4d\n", self->port, index);
                                tryAgain++;
                            }
                        }
                    }
                    tryAgain = 1;
                }
            }
        }
        sleep(HEARTBEAT_INTERVAL);
        if (debugIsOn) printf("StreamServer HeartBeat Sent Complete. Port: %.4d, Total Clients Connected: %.2d\n", self->port, totalClients);
    }

    self->hearbeatRunning = false;

    if (debugIsOn) printf("StreamServer HeartBeat End. Port: %.4d\n", self->port);
}

template<class T>
int StreamServer<T>::GetTotalConnectedClients() {
    int totalConnections = 0;

    for (int index = 0; index <= this->maxFd; ++index) {
        if (index != this->server) {
            if (FD_ISSET(index, &this->masterSet)) {
                totalConnections++;
            }
        }
    }

    return totalConnections;
}

template<class T>
StreamServer<T>::~StreamServer() {
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
    pthread_cancel(this->thHeartBeat);

    if (debugIsOn) printf("StreamServer Destroy. Port: %.4d\n", this->port);
}