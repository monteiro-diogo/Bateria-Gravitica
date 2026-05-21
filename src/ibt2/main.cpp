#include <Arduino.h>

const int RPWM_PIN = 25; 
const int LPWM_PIN = 26;

// Na API antiga, precisamos de definir canais (0 a 15)
const int RPWM_CHANNEL = 0;
const int LPWM_CHANNEL = 1;

const int freq = 25000;    
const int resolution = 8;  
const int dutyCycle30 = 76; // 30% de 255

void setup() {
  Serial.begin(115200);


  // 1. Configurar o canal com a frequência e resolução
  ledcSetup(RPWM_CHANNEL, freq, resolution);
  ledcSetup(LPWM_CHANNEL, freq, resolution);

  // 2. Associar o pino ao canal configurado
  ledcAttachPin(RPWM_PIN, RPWM_CHANNEL);
  ledcAttachPin(LPWM_PIN, LPWM_CHANNEL);

  Serial.println("Teste a 30% iniciado.");
}

void loop() {
  
  Serial.println("RPWM ativo a 30% (GPIO 25)");
  // Na API antiga, escreves para o CANAL, não para o pino
  ledcWrite(LPWM_CHANNEL, 0);
  ledcWrite(RPWM_CHANNEL, dutyCycle30);
  
  // Atraso de 5 segundos obrigatório para não fritar o driver e ver os prints
  delay(5000);

  // Ativa LPWM a 30% e desliga RPWM
  Serial.println("LPWM ativo a 30% (GPIO 26)");
  ledcWrite(RPWM_CHANNEL, 0);
  ledcWrite(LPWM_CHANNEL, dutyCycle30);

  // Atraso de 5 segundos
  delay(5000);
}