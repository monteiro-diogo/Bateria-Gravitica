#include <Arduino.h>
#include "comms.h"
#include "ina219.h"
#include "config.h"

const int ID_MINI = 2;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  delay(DELAY_ARRANQUE_MS);
  
  iniciarINA219();
  initComms(); 
  
  Serial.println("Mini " + String(ID_MINI) + " arrancou!");
}

void loop() {
  static unsigned long ultimoEnvio = 0;

  if (millis() - ultimoEnvio >= INTERVALO_TELEMETRIA_MS) {  
    ultimoEnvio = millis();
    
    // Mover a leitura para dentro do IF evita a sobrecarga contínua do I2C e CPU
    DadosEnergia dados = lerDadosINA219();
    
    sendData(ID_MINI, dados.tensao_V, dados.corrente_mA, dados.potencia_mW);
    Serial.printf("MINI %d || %.2f V | %.2f mA | %.2f mW\n", ID_MINI, dados.tensao_V, dados.corrente_mA, dados.potencia_mW);
  }
}