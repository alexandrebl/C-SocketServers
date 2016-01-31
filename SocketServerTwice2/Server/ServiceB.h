#ifndef SERVICEB_H
#define	SERVICEB_H

#include "StreamServer.h"

class ServiceB : public StreamServer{
public:
    ServiceB();
    int ReceivedData(char *buffer, int size);
};

#endif	/* SERVICEB_H */

