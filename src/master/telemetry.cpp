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
    Serial.printf("MASTER || %.2f V | %.2f mA | %.2f mW\n", dados_master.tensao_V, dados_master.corrente_mA, dados_master.potencia_mW);

    // Mostra as leituras do Mini 1
    Serial.print("MINI 1 || "); 
    if (isMini1Online()) {
      struct_message m1 = getDadosMini1();
      Serial.printf("%.2f V | %.2f mA | %.2f mW\n", m1.tensao_V, m1.corrente_mA, m1.potencia_mW);
    } else {
      Serial.println("OFFLINE");
    }

    // Mostra as leituras do Mini 2
    Serial.print("MINI 2 || ");
    if (isMini2Online()) {
      struct_message m2 = getDadosMini2();
      Serial.printf("%.2f V | %.2f mA | %.2f mW\n", m2.tensao_V, m2.corrente_mA, m2.potencia_mW);
    } else {
      Serial.println("OFFLINE");
    }
    Serial.println("------------------------------------");
  }
}