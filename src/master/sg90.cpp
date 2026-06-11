#include "sg90.h"
#include <Arduino.h>

/* Ligação do Servo:
- Sinal: GPIO 15 (PWM)
- Alimentação: 5V
- Terra: Comum com a ESP32
*/

#define PIN_PWM 15 
#define SERVO_CANAL 0 // Canal PWM

bool iniciarSG90() {
  ledcSetup(SERVO_CANAL, 50, 16); // 50Hz (padrão servo), 16 bits
  ledcAttachPin(PIN_PWM, SERVO_CANAL);

  return true;
}

void moverSG90(int angulo) {
  int duty = map(angulo, 0, 180, 1638, 8192); // Valores de duty podem variar conforme o servo
  ledcWrite(SERVO_CANAL, duty);
}

void testarSG90() {
  for (int pos = 0; pos <= 180; pos++) {
    moverSG90(pos);
    delay(15); // Ligeiramente superior a 10ms para evitar vibrações
  }
  
  delay(500); // Pausa no limite

  for (int pos = 180; pos >= 0; pos--) {
    moverSG90(pos);
    delay(15);
  }
  
  delay(500); // Pausa no limite
}