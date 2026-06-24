#include <Arduino.h>
#include "web_server.h"
#include "comms.h"
#include "config.h"
#include "telemetry.h"
#include "hardware.h"
#include "sg90.h"
#include "ibt2.h" 
#include "protocol.h" 

// ==============================================================================
// CONFIGURAÇÕES DA REDE ELÉTRICA
// ==============================================================================

const float PRODUCAO_VIRTUAL_mW = 72000.0; 

// LIMITES DE CONSUMO (WATTS)
const float LIMITE_APAGAO_mW = 5; 
const float LIMITE_NOITE_mW  = 180;  // Até 180mW é Noite
const float LIMITE_MANHA_mW  = 900;  // De 180mW até 900mW é Manhã (Atualizado)

enum FaseDoDia { APAGAO, NOITE, MANHA, TARDE };
FaseDoDia faseAtual = APAGAO;
const char* nomesFases[] = {"APAGAO", "NOITE", "MANHA", "TARDE"};

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) { delay(10); } 
  
  delay(DELAY_ARRANQUE_MS);

  iniciarWebServer(); 
  initComms(); 
  iniciarHardware(); 

  Serial.println("\n[SISTEMA] Smart Grid Online! Lógica Inversa da Manhã Ativa.\n");
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); 

  unsigned long tempoAtual = millis();

  // 1. LEITURA DOS SENSORES
  struct_message m1 = getDadosMini1(); 
  struct_message m3 = getDadosMini3(); 

  float consumo_cidade_mW = m1.potencia_mW;
  float balanco_rede_mW = PRODUCAO_VIRTUAL_mW - consumo_cidade_mW; 

  bool maxCima = (m3.distancia_cima_cm > 0.0 && m3.distancia_cima_cm <= 10.0);
  bool maxBaixo = (m3.distancia_baixo_cm > 0.0 && m3.distancia_baixo_cm <= 10.0);

  // 2. IDENTIFICAR A FASE DO DIA
  FaseDoDia novaFase;
  if (consumo_cidade_mW < LIMITE_APAGAO_mW) novaFase = APAGAO;
  else if (consumo_cidade_mW <= LIMITE_NOITE_mW) novaFase = NOITE;
  else if (consumo_cidade_mW <= LIMITE_MANHA_mW) novaFase = MANHA;
  else novaFase = TARDE;

  if (novaFase != faseAtual) {
    Serial.printf("\n >>> MUDANÇA DE ESTADO: %s (%.3fW) <<<\n", nomesFases[novaFase], consumo_cidade_mW);
    faseAtual = novaFase;
  }

  // 3. LÓGICA DE CONTROLO
  int dutyMotor = 0;
  int dutyDissipacao = 0;
  bool motorSobe = true;
  bool travar = false; // Variável interna lógica (mantida para os logs)

  switch (faseAtual) {
    case APAGAO:
      motorSobe = false; travar = true;
      dutyMotor = 30;
      if (maxBaixo) { dutyMotor = 0; travar = false; }
      break;

    case NOITE:
      // 🚀 VELOCIDADE ALTA: Sobe a carga (Mapeado de 70% a 100%)
      motorSobe = true; 
      travar = false;
      dutyMotor = map(consumo_cidade_mW, 5, 180, 70, 100);
      
      if (maxCima) { dutyMotor = 0; dutyDissipacao = 100; }
      break;

    case MANHA:
      // 🔄 LÓGICA INVERTIDA: Menor consumo = Maior velocidade (De 60% a 30%)
      motorSobe = false; 
      travar = false;
      
      // Mapeamento ajustado para o novo teto de 900mW:
      // Consumo perto de 180mW -> Motor a 60% (Mais rápido)
      // Consumo perto de 900mW -> Motor a 30% (Mais lento)
      dutyMotor = map(consumo_cidade_mW, 180, 900, 60, 30);
      
      dutyDissipacao = (balanco_rede_mW > 0) ? map(balanco_rede_mW * 100, 0, PRODUCAO_VIRTUAL_mW * 100, 0, 100) : 0;
      
      if (maxBaixo) { dutyMotor = 0; }
      break;

    case TARDE:
      // 🛑 PARADO: Motor totalmente desligado na fase da tarde
      motorSobe = false; 
      dutyMotor = 0; 
      dutyDissipacao = 100; 
      travar = true; 
      break;
  }

  // 4. EXECUÇÃO DE COMANDOS
  motorIBT2(constrain(dutyMotor, 0, 100), motorSobe);
  controlarDissipacao(constrain(dutyDissipacao, 0, 100));
  
  // Servo fixo em 0 (Solto) conforme solicitado
  moverSG90(0);

  // Debug visual no terminal
  static unsigned long ultimoPrintLogs = 0;
  if (tempoAtual - ultimoPrintLogs > 2000) {
    Serial.printf("[REDE] Fase: %s | Consumo: %.3fW | Motor: %d%% (%s) | Servo: PARADO\n", 
                  nomesFases[faseAtual], consumo_cidade_mW, dutyMotor, (dutyMotor == 0 ? "PARADO" : (motorSobe ? "Sobe" : "Desce")));
    ultimoPrintLogs = tempoAtual;
  }

  delay(50); 
}