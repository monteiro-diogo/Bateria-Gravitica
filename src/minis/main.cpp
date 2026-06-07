#include <Arduino.h>
#include "comms.h"
#include "ina226.h"
#include "config.h"

const int ID_MINI = 1; 

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  while (!Serial && millis() < 5000) {
    delay(10);
  }

  delay(DELAY_ARRANQUE_MS);
  
  iniciarINA226();
  initComms(); 
  
  Serial.println("Mini " + String(ID_MINI) + " arrancou!");
}

// Loop principal: Lê os dados do sensor e envia por ESP-NOW de tempo a tempo definido em config.h
void loop() {
  static unsigned long ultimoEnvio = 0;
  
  if (millis() - ultimoEnvio >= INTERVALO_TELEMETRIA_MS) {  
    ultimoEnvio = millis();
    DadosEnergia dados = lerDadosINA226();
    sendData(ID_MINI, dados.tensao_V, dados.corrente_mA, dados.potencia_mW);
  }
}