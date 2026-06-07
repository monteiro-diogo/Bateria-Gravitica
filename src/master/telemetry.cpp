#include "telemetry.h"
#include <Arduino.h>
#include "comms.h"
#include "ina219.h"
#include "config.h"

// Função para imprimir todas as leituras (Master + Minis) na consola série
void imprimirTelemetria() {
  static unsigned long ultimoEnvio = 0;
  
  if (millis() - ultimoEnvio >= INTERVALO_TELEMETRIA_MS) {  
    ultimoEnvio = millis();
    
    Serial.println("------------------------------------");
    // Mostra as leituras do Master
    DadosEnergia dados_master = lerDadosINA219();
    Serial.print("MASTER || ");
    Serial.print(dados_master.tensao_V); Serial.print(" V | ");
    Serial.print(dados_master.corrente_mA); Serial.print(" mA | ");
    Serial.print(dados_master.potencia_mW); Serial.println(" mW");

    // Mostra as leituras do Mini 1
    struct_message m1 = getDadosMini1();
    Serial.print("MINI 1 || "); 
    Serial.print(m1.tensao_V); Serial.print(" V | ");
    Serial.print(m1.corrente_mA); Serial.print(" mA | ");
    Serial.print(m1.potencia_mW); Serial.println(" mW");

    // Mostra as leituras do Mini 2
    struct_message m2 = getDadosMini2();
    Serial.print("MINI 2 || ");
    Serial.print(m2.tensao_V); Serial.print(" V | ");
    Serial.print(m2.corrente_mA); Serial.print(" mA | ");  
    Serial.print(m2.potencia_mW); Serial.println(" mW");
    Serial.println("------------------------------------");
  }
}