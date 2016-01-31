/* 
 * File:   StreamServer.h
 * Author: alexandre
 *
 * Created on 4 de Março de 2014, 09:34
 */

#ifndef TCPSERVERSOCKET_H
#define	TCPSERVERSOCKET_H

#include <cstdlib>
#include <csignal>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cerrno>
#include <sys/types.h> 
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#define HEARTBEAT_INTERVAL      2     // Segundos
//#define ECHO_TEST
#define ECHO_TEST_ONLYONE

class StreamServer {
public:
    void initService(
        int Port, int BufferSize = 8192, int MaxListenConnections = 10, bool DebugIsOn = false);
    virtual ~StreamServer();
    int Write(char *buffer, int size);
    int Write(int fd, char *buffer, int size);
    virtual int ReceivedData(char *buffer, int size) = 0;
    
private:
    int server;
    int port;    
    int bufferSize;
    int maxListenConnections;
    int maxFd;
    bool debugIsOn;
    fd_set masterSet;
    
    //Accept and Read    
    pthread_t thAcceptClients;
    static void* AccpetClients(void* arg);
    void AccpetClientsEvent();
    
    //Control
    static bool running;
    bool acceptRunning;
};

#endif	/* TCPSERVERSOCKET_H */

