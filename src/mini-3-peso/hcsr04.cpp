#include "hcsr04.h"
#include <Arduino.h>

HCSR04::HCSR04(int trig, int echo) {
  trigPin = trig;
  echoPin = echo;
}

void HCSR04::iniciar() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW); // Estado inicial seguro
}

float HCSR04::lerDistanciaCm() {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Pulso de 10us
  digitalWrite(trigPin, LOW);

  // Medir o tempo do eco (Timeout de 30ms)
  long duration = pulseIn(echoPin, HIGH, 30000);

  // Calcular a distância
  float distancia = (duration * 0.0343) / 2.0;

  // Validação (timeout ou fora de alcance)
  if (duration == 0 || distancia > 400.0) {
    return -1.0; 
  }

  return distancia;
}