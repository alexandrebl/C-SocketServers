#ifndef SERVICEA_H
#define	SERVICEA_H

#include "StreamServer.h"

class ServiceA : public StreamServer{
public:
    ServiceA();
    int ReceivedData(char *buffer, int size);
};

#endif	/* SERVICEA_H */

