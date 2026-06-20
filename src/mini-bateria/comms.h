#ifndef COMMS_H
#define COMMS_H
#include "protocol.h"

void initComms();
void sendData(int id, float dist_cima, float dist_baixo);

#endif