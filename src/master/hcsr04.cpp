#include "hcsr04.h"
#include <Arduino.h>

/* Ligação do HC-SR04:
- Trigger: GPIO 5
- Echo: GPIO 18
- Alimentação: 5V e GND comum com a ESP32
*/

// Definição dos pinos do sensor ultrassónico HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// Função para iniciar o sensor 
bool iniciarUltrassom() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW); // Estado inicial seguro
  
  return true;
}

// Função para ler a distância
float lerDistanciaCm() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10); // Gerar o pulso de 10 microsegundos
  digitalWrite(TRIG_PIN, LOW);

  // Medir o tempo do eco (Timeout de 30ms para não bloquear a ESP32 muito tempo)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Calcular a distância (velocidade do som: 0.0343 cm/us)
  float distancia = (duration * 0.0343) / 2.0;

  // Validação e tratamento de erros (timeout)
  if (duration == 0) {
    return -1.0; 
  }

  // Filtro de segurança para evitar falsos positivos
  if (distancia > 400.0) {
    return -1.0; 
  }

  return distancia;
}