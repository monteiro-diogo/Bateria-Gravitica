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

const float PRODUCAO_VIRTUAL_W = 4.0; 

// LIMITES POR CONSUMO REAL (WATTS)
const float LIMITE_APAGAO_W = 0.005; 
const float LIMITE_NOITE_W  = 0.04;  
const float LIMITE_MANHA_W  = 0.22;  

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
  iniciarDissipacao(); 
  iniciarSG90(); // Inicializa o Servo no Pino 15

  Serial.println("\n[SISTEMA] Smart Grid Online! Sistema com Trava Ativo.\n");
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); 

  unsigned long tempoAtual = millis();

  // 1. LEITURA DOS SENSORES
  struct_message m1 = getDadosMini1(); 
  struct_message m3 = getDadosMini3(); 

  float consumo_cidade_W = (m1.tensao_V * m1.corrente_mA) / 1000.0;
  float balanco_rede_W = PRODUCAO_VIRTUAL_W - consumo_cidade_W; 

  bool maxCima = (m3.distancia_cima_cm > 0.0 && m3.distancia_cima_cm <= 10.0);
  bool maxBaixo = (m3.distancia_baixo_cm > 0.0 && m3.distancia_baixo_cm <= 10.0);

  // 2. IDENTIFICAR A FASE DO DIA
  FaseDoDia novaFase;
  if (consumo_cidade_W < LIMITE_APAGAO_W) novaFase = APAGAO;
  else if (consumo_cidade_W <= LIMITE_NOITE_W) novaFase = NOITE;
  else if (consumo_cidade_W <= LIMITE_MANHA_W) novaFase = MANHA;
  else novaFase = TARDE;

  if (novaFase != faseAtual) {
    Serial.printf("\n >>> MUDANÇA DE ESTADO: %s (%.3fW) <<<\n", nomesFases[novaFase], consumo_cidade_W);
    faseAtual = novaFase;
  }

  // 3. LÓGICA DE CONTROLO
  int dutyMotor = 0;
  int dutyDissipacao = 0;
  bool motorSobe = true;
  bool travar = false; // Estado do servo

  switch (faseAtual) {
    case APAGAO:
      motorSobe = false; travar = true;
      dutyMotor = 30;
      if (maxBaixo) { dutyMotor = 0; travar = false; }
      break;

    case NOITE:
      motorSobe = true; travar = false;
      dutyMotor = map(balanco_rede_W * 100, 0, PRODUCAO_VIRTUAL_W * 100, 20, 100);
      if (maxCima) { dutyMotor = 0; dutyDissipacao = 100; }
      break;

    case MANHA:
      dutyMotor = 0; travar = false;
      dutyDissipacao = (balanco_rede_W > 0) ? map(balanco_rede_W * 100, 0, PRODUCAO_VIRTUAL_W * 100, 0, 100) : 0;
      break;

    case TARDE:
      motorSobe = false; travar = true;
      dutyDissipacao = 100;
      dutyMotor = map(consumo_cidade_W * 1000, 220, 300, 30, 100);
      if (maxBaixo) { dutyMotor = 0; travar = false; }
      break;
  }

  // 4. EXECUÇÃO DE COMANDOS
  motorIBT2(constrain(dutyMotor, 0, 100), motorSobe);
  controlarDissipacao(constrain(dutyDissipacao, 0, 100));
  
  // Atuação do Servo (Trava): 30 graus para travar, 0 para libertar
  moverSG90(travar ? 30 : 0);

  // Debug visual
  static unsigned long ultimoPrintLogs = 0;
  if (tempoAtual - ultimoPrintLogs > 2000) {
    Serial.printf("[REDE] Fase: %s | Motor: %d%% (%s) | Trava: %s\n", 
                  nomesFases[faseAtual], dutyMotor, (motorSobe ? "Sobe" : "Desce"), (travar ? "ATIVA" : "SOLTA"));
    ultimoPrintLogs = tempoAtual;
  }

  delay(50); 
}