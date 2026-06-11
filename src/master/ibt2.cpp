#include "ibt2.h"
#include <Arduino.h>

/* Ligação do Driver IBT-2:
- RPWM: GPIO 25 (PWM Horário)
- LPWM: GPIO 26 (PWM Anti-horário)
- R_EN e L_EN: Ligados a 5V (ou a pinos GPIO em HIGH)
- Alimentação: 5V (VCC) e GND Comum com a ESP32
*/

#define RPWM_PIN 25 
#define LPWM_PIN 26

// Definição dos canais PWM (exigidos na versão antiga do core ESP32)
#define RPWM_CANAL 0 
#define LPWM_CANAL 1 

#define FREQ_HZ 25000     // Frequência de 25kHz recomendada para o IBT-2
#define RESOLUTION_BITS 8 // Resolução de 8 bits (valores de duty de 0 a 255)

const int dutyCycle30 = 76;

bool iniciarIBT2() {
  ledcSetup(RPWM_CANAL, FREQ_HZ, RESOLUTION_BITS);
  ledcSetup(LPWM_CANAL, FREQ_HZ, RESOLUTION_BITS);

  ledcAttachPin(RPWM_PIN, RPWM_CANAL);
  ledcAttachPin(LPWM_PIN, LPWM_CANAL);

  return true;
}

void moverIBT2(int dutyRPWM, int dutyLPWM) {
  ledcWrite(RPWM_CANAL, dutyRPWM);
  ledcWrite(LPWM_CANAL, dutyLPWM);
}

void testarIBT2() {
  Serial.println("RPWM ativo a 30% (Canal 0)");
  moverIBT2(dutyCycle30, 0);
  delay(5000); 

  Serial.println("LPWM ativo a 30% (Canal 1)");
  moverIBT2(0, dutyCycle30);
  delay(5000); 
}