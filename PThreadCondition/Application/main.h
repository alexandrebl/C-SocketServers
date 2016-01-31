/* 
 * File:   main.h
 * Author: alexandre
 *
 * Created on 3 de Março de 2014, 23:15
 */

#ifndef MAIN_H
#define	MAIN_H

#include <cstdlib>
#include <csignal>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

class Application{
public:
    void start();
    void init();
    void end();
    
    //Signal handle e wait condition
    pthread_mutex_t mutexWait;
    static pthread_cond_t condWait;
    static void signalHandler(int signum);
};

#endif	/* MAIN_H */

