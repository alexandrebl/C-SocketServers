/* 
 * File:   StackBuffer.cpp
 * Author: alexandre
 * 
 * Created on 10 de Março de 2014, 08:53
 */

#include "StackBuffer.h"

StackBuffer::StackBuffer(int Size, const char *Tag, bool DebugIsOn) {
    this->bufferSize = Size;
    this->debugIsOn = DebugIsOn;
    this->dataBuffer = (char*) malloc(this->bufferSize);
    this->tag = (char*) Tag;

    if (this->debugIsOn) printf("StackBuffer %s init\n", this->tag);

    pthread_mutex_init(&this->opctrl, NULL);

    this->running = true;

    if ((this->bufferSize > 0) && (this->bufferSize <= BUFFER_SIZE_MAX)) {
        if (this->debugIsOn) printf("StackBuffer %s initialized\n", this->tag);
    } else {
        if (this->debugIsOn) printf("StackBuffer %s, buffer size error. Size:%.4d\n", this->tag, this->bufferSize);
        end(true);
    }
}

StackBuffer::StackBuffer() {
}

void StackBuffer::init(int Size, const char *Tag, bool DebugIsOn) {
    if (this->running == false) {
        this->bufferSize = Size;
        this->debugIsOn = DebugIsOn;
        this->dataBuffer = (char*) malloc(this->bufferSize);
        this->tag = (char*) Tag;
        this->dataSize = 0;

        if (this->debugIsOn) printf("StackBuffer %s init\n", this->tag);

        pthread_mutex_init(&this->opctrl, NULL);

        this->running = true;

        if ((this->bufferSize > 0) && (this->bufferSize <= BUFFER_SIZE_MAX)) {
            if (this->debugIsOn) printf("StackBuffer %s initialized\n", this->tag);
        } else {
            if (this->debugIsOn) printf("StackBuffer %s, buffer size error. Size:%.4d\n", this->tag, this->bufferSize);
            end(true);
        }
    }
}

StackBuffer::~StackBuffer() {
    this->end(false);
}

bool StackBuffer::set(char *buffer, int size) {
    bool result = false;

    this->lock();
    
    if (size > 0) {
        if (this->debugIsOn) printf("StackBuffer %s, try to set. Buffer Size: %.4d, Data Size: %.4d, Size: %.4d\n",
                this->tag, this->bufferSize, this->dataSize, size);
        if ((this->dataSize + size) > this->bufferSize) {
            if (this->debugIsOn) printf("StackBuffer %s error, buffer limit. Buffer Size: %.4d, Data Size: %.4d, Size: %.4d\n",
                    this->tag, this->bufferSize, this->dataSize, size);
        } else {
            memcpy(&this->dataBuffer[this->dataSize], buffer, size);
            this->dataSize += size;
            if (this->debugIsOn) printf("StackBuffer %s, setted. Buffer Size: %.4d, Data Size: %.4d, Size: %.4d\n",
                    this->tag, this->bufferSize, this->dataSize, size);
        }
    } else {
        if (this->debugIsOn) printf("StackBuffer %s error, not set. Size: %.4d.\n", this->tag, size);
    }

    this->unlock();
    return result;
}

int StackBuffer::get(char *buffer) {
    this->lock();
    if (this->debugIsOn) printf("StackBuffer %s, try to get. Data Size: %.4d\n", this->tag, this->dataSize);

    int size = 0;

    if (this->dataSize > 0) {
        size = this->dataSize;

        memset(buffer, 0, size);
        memcpy(buffer, this->dataBuffer, size);
        this->dataSize = 0;
        memset(this->dataBuffer, 0, this->bufferSize);
    }

    if (this->debugIsOn) printf("StackBuffer %s, get end. Size: %.4d\n", this->tag, size);
    this->unlock();

    return size;
}

void StackBuffer::end(bool failure) {
    if (this->running == true) {
        free(this->dataBuffer);
        pthread_mutex_unlock(&this->opctrl);
        pthread_mutex_destroy(&this->opctrl);
        if (this->debugIsOn) printf("StackBuffer %s end\n", this->tag);
        if (failure) {
            if (this->debugIsOn) printf("StackBuffer %s, exit by failure\n", this->tag);
            exit(EXIT_FAILURE);
        }
    }
}

void StackBuffer::lock() {
    if (this->running == true) {
        pthread_mutex_lock(&this->opctrl);
        if (this->debugIsOn) printf("StackBuffer %s, lock is on\n", this->tag);
    }
}

void StackBuffer::unlock() {
    if (this->running == true) {
        pthread_mutex_unlock(&this->opctrl);
        if (this->debugIsOn) printf("StackBuffer %s, lock is off\n", this->tag);
    }
}

