#ifndef COMMS_H
#define COMMS_H
#include "protocol.h"

// Funções para iniciar e aceder aos dados guardados
void initComms();
// Funções "Getter" para o web_server.cpp ir buscar os dados de forma segura
struct_message getDadosMini1();
struct_message getDadosMini2();
struct_message getDadosMini3();

#endif