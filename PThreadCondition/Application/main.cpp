/* 
 * File:   main.cpp
 * Author: alexandre
 *
 * Created on 3 de Março de 2014, 23:12
 */

#include "main.h"

using namespace std;

pthread_cond_t Application::condWait;
        
void Application::start(){
    puts("Application start()");
    pthread_mutex_init(&mutexWait, NULL);    
    pthread_cond_init(&condWait, NULL);
    pthread_mutex_lock(&mutexWait);
    puts("CondWait");
    pthread_cond_wait(&condWait, &mutexWait);
    puts("CondWait End");
    this->end();
}

void Application::init(){

    puts("Application init()");
}


void Application::end(){
    puts("Application end()");
    pthread_mutex_unlock(&mutexWait);
    pthread_cond_destroy(&condWait);
    pthread_mutex_destroy(&mutexWait);
}

void Application::signalHandler(int signum){
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
    pthread_cond_signal(&Application::condWait);
}

int main() {

    Application server;
    
    puts("Application Run");
    signal(SIGINT, Application::signalHandler); 
    signal(SIGABRT, Application::signalHandler); 
    signal(SIGILL, Application::signalHandler); 
    signal(SIGSEGV, Application::signalHandler); 
    signal(SIGTERM, Application::signalHandler); 
    signal(SIGFPE, Application::signalHandler);
    server.init();
    server.start();
    puts("Application End");
    
    return 0;
}

