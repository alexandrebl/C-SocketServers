/* 
 * File:   main.cpp
 * Author: alexandre
 *
 * Created on 3 de Março de 2014, 23:12
 */

#include "main.h"

using namespace std;

pthread_cond_t Server::condWait;
        
void Server::start(){
    puts("Server start()");
    pthread_mutex_init(&mutexWait, NULL);    
    pthread_cond_init(&condWait, NULL);
    pthread_mutex_lock(&mutexWait);
    puts("CondWait");
    pthread_cond_wait(&condWait, &mutexWait);
    puts("CondWait End");
    this->end();
}

void Server::init(){
    puts("Server init()");
    //Init Service A
    this->serviceA = new ServiceA();
    //Init Service B
    this->serviceB = new ServiceB();
}

void Server::end(){    
    pthread_mutex_unlock(&mutexWait);
    pthread_cond_destroy(&condWait);
    pthread_mutex_destroy(&mutexWait);
    
    puts("Destroy Service A");
    delete this->serviceA;
    
    puts("Destroy Service B");
    delete this->serviceB;
    
    puts("Server end()");
}

void Server::signalHandler(int signum){
    switch(signum){
        case SIGINT : {
            puts("Received SIGINT - Receipt of an interactive attention signal.");
            break;
        }
        case SIGABRT : {
            puts("Received SIGABRT - Abnormal termination of the program, such as a call to abort.");
            break;
        }
        case SIGILL : {
            puts("Received SIGILL - Detection of an illegal instruction.");
            break;
        }
        case SIGSEGV : {
            puts("Received SIGSEGV - An invalid access to storage.");
            break;
        }
        case SIGTERM : {
            puts("Received SIGTERM - A termination request sent to the program.");
            break;
        }
        case SIGFPE : {
            puts("Received SIGFPE - An erroneous arithmetic operation, such as a divide by zero or an operation resulting in overflow.");
            break;
        }
    }    
    pthread_cond_signal(&Server::condWait);
}

int main() {

    Server *server = new Server();
    
    puts("Application Init");
    signal(SIGINT, Server::signalHandler); 
    signal(SIGABRT, Server::signalHandler); 
    signal(SIGILL, Server::signalHandler); 
    signal(SIGSEGV, Server::signalHandler); 
    signal(SIGTERM, Server::signalHandler); 
    signal(SIGFPE, Server::signalHandler);
    server->init();
    server->start();
    delete server;
    puts("Application End");
    
    return 0;
}

