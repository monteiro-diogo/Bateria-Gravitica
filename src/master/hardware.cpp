#include "ina219.h"
#include "hcsr04.h"
#include "sg90.h"
#include "ibt2.h"
#include <Arduino.h>

void iniciarHardware() {
  Serial.println("\n--- A INICIAR HARDWARE ---");

  // INA219
  if (!iniciarINA219()) {
    Serial.println("[INA219] AVISO: Falha ao iniciar INA219! (Verifica cabos I2C). Continua...");
    // Removido o while(1) para não congelar o sistema
  } else { 
    Serial.println("[INA219] Iniciado com sucesso."); 
  }

  // HC-SR04
  if (!iniciarUltrassom()) {
    Serial.println("[HC-SR04] AVISO: Falha ao iniciar Ultrassom! Continua...");
    // Removido o while(1)
  } else { 
    Serial.println("[HC-SR04] Iniciado com sucesso."); 
  }

  // SG90
  if (!iniciarSG90()) {
    Serial.println("[SG90] AVISO: Falha ao iniciar SG90! Continua...");
    // Removido o while(1)
  } else { 
    Serial.println("[SG90] Iniciado com sucesso."); 
  }

  // IBT-2
  if (!iniciarIBT2()) {
    Serial.println("[IBT-2] ERRO: Falha ao iniciar IBT-2!");
    // Aqui podes deixar ou tirar, mas como o motor é o principal, convém saber se falhou.
  } else { 
    Serial.println("[IBT-2] Iniciado com sucesso."); 
  }

  Serial.println("--- HARDWARE CONCLUÍDO ---");
  Serial.println("------------------------------------");
}