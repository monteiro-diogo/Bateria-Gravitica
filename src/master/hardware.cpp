#include "ina219.h"
#include "hcsr04.h"
#include "sg90.h"
#include "ibt2.h"
#include <Arduino.h>


void iniciarHardware() {
  // INA219
  if (!iniciarINA219()) {
    Serial.println("[INA219] ERRO: Falha ao iniciar INA219!");
    while (1) { delay(10); } 
  } else { Serial.println("[INA219] Iniciado com sucesso."); }

  // HC-SR04
  if (!iniciarUltrassom()) {
    Serial.println("[HC-SR04] ERRO: Falha ao iniciar Ultrassom!");
    while (1) { delay(10); } 
  } else { Serial.println("[HC-SR04] Iniciado com sucesso."); }

  // SG90
  if (!iniciarSG90()) {
    Serial.println("[SG90] ERRO: Falha ao iniciar SG90!");
    while (1) { delay(10); } 
  } else { Serial.println("[SG90] Iniciado com sucesso."); }

  // IBT-2
  if (!iniciarIBT2()) {
    Serial.println("[IBT-2] ERRO: Falha ao iniciar IBT-2!");
    while (1) { delay(10); } 
  } else { Serial.println("[IBT-2] Iniciado com sucesso."); }

  Serial.println("------------------------------------");
}