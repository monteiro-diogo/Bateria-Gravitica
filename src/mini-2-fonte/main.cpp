#include <Arduino.h>
#include "comms.h"
#include "ina226.h"
#include "config.h"

const int ID_MINI = 2;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  delay(DELAY_ARRANQUE_MS);
  
  iniciarINA226();
  initComms(); 
  
  Serial.println("Mini " + String(ID_MINI) + " arrancou!");
}

// Loop principal: Lê os dados do sensor e envia por ESP-NOW de tempo a tempo definido em config.h
void loop() {
  static unsigned long ultimoEnvio = 0;
  DadosEnergia dados = lerDadosINA226();

  if (millis() - ultimoEnvio >= INTERVALO_TELEMETRIA_MS) {  
    ultimoEnvio = millis();
    sendData(ID_MINI, dados.tensao_V, dados.corrente_mA, dados.potencia_mW);
    Serial.printf("MINI %d || %.2f V | %.2f mA | %.2f mW\n", ID_MINI, dados.tensao_V, dados.corrente_mA, dados.potencia_mW);
  }
}