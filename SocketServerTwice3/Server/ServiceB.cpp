#include "ServiceB.h"
#include <stdio.h>
#include <stdlib.h>

ServiceB::ServiceB(){
    this->initService(2001, 8192, 10, true); //Port, Buffer Size, Connections, HeartBeat
}


int ServiceB::ReceivedData(char *buffer, int size){
    
    printf("Service B - Received Data %.4d bytes\n", size);
    
    return size;
}