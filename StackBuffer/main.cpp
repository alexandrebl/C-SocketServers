/* 
 * File:   main.cpp
 * Author: alexandre
 *
 * Created on 10 de Março de 2014, 08:51
 */

#include <cstdlib>
#include "StackBuffer.h"

using namespace std;

int main() {

    int bufferSize = 32;
    
    char fraseA[5] = { 'A', 'B', 'C', 'D', 'E' };
    char fraseB[5] = { '1', '2', '3', '4', '5' };
    char fraseC[bufferSize];
    
    StackBuffer *buffer = new StackBuffer(bufferSize, "MainBuffer", true);
    
    buffer->set(fraseA, sizeof(fraseA));
    buffer->set(fraseB, sizeof(fraseB));
    buffer->set(fraseA, sizeof(fraseA));
    buffer->set(fraseB, sizeof(fraseB));
    buffer->set(fraseA, sizeof(fraseA));
    buffer->set(fraseB, sizeof(fraseB));
    buffer->set(fraseA, sizeof(fraseA));
    buffer->set(fraseB, sizeof(fraseB));
    buffer->set(fraseA, sizeof(fraseA));
    buffer->set(fraseB, sizeof(fraseB));
    buffer->set(fraseA, sizeof(fraseA));
    buffer->set(fraseB, sizeof(fraseB));
    
    int size = buffer->get(fraseC);
    
    printf("Size: %.4d\n", size);
    for(int pos = 0; pos < size; pos++){
        printf("%.2X ", fraseC[pos]);
    }
    printf("\n");
    for(int pos = 0; pos < size; pos++){
        printf("%c ", fraseC[pos]);
    }
    printf("\n");

    delete buffer;

    return 0;
}


