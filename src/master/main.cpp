#include <Arduino.h>
#include "ina219.h"
#include "web_server.h"
#include "comms.h"
#include "config.h"

// Configurações iniciais do sistema
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(DELAY_ARRANQUE_MS);
  Serial.println("\n--- Configuracao WI-FI ---");

  iniciarWebServer(); // Inicia a rede AP
  initComms();        // Inicia o recetor ESP-NOW (DEPOIS DA REDE)
  
  Serial.println("Sistema online!");
  Serial.println("----------------------------------------------");

  if (!iniciarSensor()) {
    Serial.println("ERRO: Sensor INA219 nao encontrado!");
    while (1) { delay(10); } 
  }
}

// Loop principal: Processa o web server e mostra os dados a cada segundo
void loop() {
  processarWebServer();
  
  static unsigned long ultimoEnvio = 0;
  if (millis() - ultimoEnvio >= INTERVALO_TELEMETRIA_MS) {  
    ultimoEnvio = millis();
    
    // Mostra as leituras locais
    DadosEnergia dados_master = lerDadosSensor();
    Serial.print("MASTER -> ");
    Serial.print(dados_master.tensao_V); Serial.print(" V | ");
    Serial.print(dados_master.corrente_mA); Serial.print(" mA | ");
    Serial.print(dados_master.potencia_mW); Serial.println(" mW");

    // Mostra as leituras recebidas por ESP-NOW
    struct_message m1 = getDadosMini1();
    Serial.print("MINI 1 -> "); 
    Serial.print(m1.tensao_V); Serial.print(" V | ");
    Serial.print(m1.corrente_mA); Serial.print(" mA | ");
     Serial.print(m1.potencia_mW); Serial.println(" mW");

    struct_message m2 = getDadosMini2();
    Serial.print("MINI 2 -> ");
    Serial.print(m2.tensao_V); Serial.print(" V | ");
    Serial.print(m2.corrente_mA); Serial.print(" mA | ");  
    Serial.print(m2.potencia_mW); Serial.println(" mW");
    Serial.println("---");
  }
}