/* 
 * File:   StackBuffer.h
 * Author: alexandre
 *
 * Created on 10 de Março de 2014, 08:53
 */

#ifndef STACKBUFFER_H
#define	STACKBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE_MAX 8192

class StackBuffer {
public:
    StackBuffer(int Size, const char *Tag, bool DebugIsOn = false);
    void init(int Size, const char *Tag, bool DebugIsOn = false);
    
    StackBuffer();
    virtual ~StackBuffer();
    
    //Operations
    bool set(char *buffer, int size);
    int get(char *buffer);
    
private:
    //Buffer
    int bufferSize;
    char *dataBuffer;
    //Operations
    int dataSize;
    //Control and Log
    bool debugIsOn;
    char *tag;
    bool running;
    
    void end(bool failure);
    
    //Mutex operations
    pthread_mutex_t opctrl;
    void lock();
    void unlock();
};

#endif	/* STACKBUFFER_H */

