#include <Arduino.h>
#include "comms.h"
#include "config.h"
#include "hcsr04.h"

const int ID_MINI = 3;

// Definição dos pinos para os sensores na ESP32-C3 Super Mini
#define TRIG_CIMA 0
#define ECHO_CIMA 1

#define TRIG_BAIXO 3
#define ECHO_BAIXO 10

// Instanciar os dois sensores
HCSR04 sensorCima(TRIG_CIMA, ECHO_CIMA);
HCSR04 sensorBaixo(TRIG_BAIXO, ECHO_BAIXO);

// Variável para controlar o envio por tempo sem usar delays bloqueantes
unsigned long ultimoTempoEnvio = 0;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(DELAY_ARRANQUE_MS);

  // Iniciar hardware
  sensorCima.iniciar();
  sensorBaixo.iniciar();
  initComms(); 
  
  Serial.println("Mini " + String(ID_MINI) + " arrancou com 2 sensores HC-SR04!");
}

void loop() {
  if (millis() - ultimoTempoEnvio >= 250) {
    ultimoTempoEnvio = millis();

    // Lê os sensores
    float distCima = sensorCima.lerDistanciaCm();
    float distBaixo = sensorBaixo.lerDistanciaCm();

    // Imprime para debug local
    Serial.print("Cima: "); Serial.print(distCima);
    Serial.print(" cm | Baixo: "); Serial.print(distBaixo);
    Serial.println(" cm");

    // Envia por ESP-NOW
    sendData(ID_MINI, distCima, distBaixo);
  }
}