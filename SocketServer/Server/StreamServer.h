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
#define ECHO_TEST

template<class T>
class StreamServer {
public:
    StreamServer(T *instance, int Port, int BufferSize = 8192, 
        int MaxListenConnections = 10, int MaxClients = 40, bool useHeartBeat = true, bool DebugIsOn = false);
    virtual ~StreamServer();
    int Write(char *buffer, int size);   
    
    T *selfClass;
    typedef void (T::*OnSocketRead)(char *buffer, int size);
    OnSocketRead ptr;
    
private:

    int server;
    int port;    
    int bufferSize;
    int maxClients;
    int maxListenConnections;
    int maxFd;
    bool debugIsOn;
    fd_set masterSet;
    
    //Accept and Read    
    pthread_t thAcceptClients;
    static void* AccpetClients(void* arg);
    void AccpetClientsEvent(void* arg);
    
    //Accept and Read    
    pthread_t thHeartBeat;
    static void* HeartBeat(void* arg);
    void HeartBeatEvent(void* arg);
    
    int GetTotalConnectedClients();
    //Control
    static bool running;
    bool acceptRunning;
    bool writeRunning;
    bool hearbeatRunning;
};

#endif	/* TCPSERVERSOCKET_H */

