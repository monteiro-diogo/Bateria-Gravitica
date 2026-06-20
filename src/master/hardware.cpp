#include "ina226.h"
#include "sg90.h"
#include "ibt2.h"
#include <Arduino.h>


void iniciarHardware() {
  // INA226
  if (!iniciarINA226()) {
    Serial.println("[INA226] ERRO: Falha ao iniciar INA219!");
  } else { Serial.println("[INA226] Iniciado com sucesso."); }


  // SG90
  if (!iniciarSG90()) {
    Serial.println("[SG90] ERRO: Falha ao iniciar SG90!");
  } else { Serial.println("[SG90] Iniciado com sucesso."); }

  // IBT-2
  if (!iniciarIBT2()) {
    Serial.println("[IBT-2] ERRO: Falha ao iniciar IBT-2!");
  } else { Serial.println("[IBT-2] Iniciado com sucesso."); }

  Serial.println("------------------------------------");
}