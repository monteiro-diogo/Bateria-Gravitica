#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <Arduino.h>

// Estrutura de dados global, partilhada entre o Master e todos os Minis
typedef struct struct_message {
  int id;
  float tensao_V;
  float corrente_mA;
  float potencia_mW;
  float distancia_cima_cm;
  float distancia_baixo_cm;
} struct_message;



#endif