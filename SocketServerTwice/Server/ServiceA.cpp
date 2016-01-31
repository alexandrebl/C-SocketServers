#include "ServiceA.h"
#include <stdio.h>
#include <stdlib.h>

ServiceA::ServiceA(){
    this->initService(2000, 8192, 10, 50, true, true); //Port, Buffer Size, Connections, HeartBeat
}


int ServiceA::ReceivedData(char *buffer, int size){
    
    printf("Service A - Received Data %.4d bytes\n", size);
    
    return size;
}