#include "ina219.h"
#include "hcsr04.h"
#include "servo.h"
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

  // Servo
  if (!iniciarServo()) {
    Serial.println("[SERVO] ERRO: Falha ao iniciar Servo!");
    while (1) { delay(10); } 
  } else { Serial.println("[SERVO] Iniciado com sucesso."); }


  Serial.println("------------------------------------");
}