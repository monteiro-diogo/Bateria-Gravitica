#include "telemetry.h"
#include <Arduino.h>
#include "comms.h"
#include "ina219.h"
#include "hcsr04.h"
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
    struct_message m1 = getDadosMini1();
    Serial.printf("MINI 1 || %.2f V | %.2f mA | %.2f mW\n", m1.tensao_V, m1.corrente_mA, m1.potencia_mW);

    // Mostra as leituras do Mini 2
    struct_message m2 = getDadosMini2();
    Serial.printf("MINI 2 || %.2f V | %.2f mA | %.2f mW\n", m2.tensao_V, m2.corrente_mA, m2.potencia_mW);
    
    float distancia = lerDistanciaCm();
    if (distancia >= 0) {
      Serial.printf("DISTANCIA || %.2f cm\n", distancia);
    } else {
      Serial.println("DISTANCIA || Erro na leitura");
    }
    Serial.println("------------------------------------");
  }
}