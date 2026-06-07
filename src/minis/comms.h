#ifndef COMMS_H
#define COMMS_H
#include "protocol.h"

void initComms();
void sendData(int id, float tensao_V, float corrente_mA, float potencia_mW);

#endif