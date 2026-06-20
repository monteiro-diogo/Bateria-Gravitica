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

// LIMITES POR CONSUMO REAL (WATTS) DEFINIDOS PELOS TESTES
const float LIMITE_APAGAO_W = 0.005; // Praticamente 0W (Usamos 0.005W para ignorar ruído do sensor)
const float LIMITE_NOITE_W  = 0.04;  // Entre ~0W e 0.04W
const float LIMITE_MANHA_W  = 0.15;  // Entre 0.04W e 0.22W
// Acima de 0.22W será considerado TARDE

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

  Serial.println("\n[SISTEMA] Smart Grid Online! A ler consumo em tempo real...\n");
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); 

  unsigned long tempoAtual = millis();

  // 1. LEITURA DOS SENSORES
  struct_message m1 = getDadosMini1(); // INA226 (Tensão e Corrente)
  struct_message m3 = getDadosMini3(); // Ultrassons (Limites Mecânicos)

  float tensao_cidade = m1.tensao_V;
  float consumo_cidade_W = (m1.tensao_V * m1.corrente_mA) / 1000.0;
  float balanco_rede_W = PRODUCAO_VIRTUAL_W - consumo_cidade_W; 

  bool maxCima = (m3.distancia_cima_cm > 0.0 && m3.distancia_cima_cm <= 10.0);
  bool maxBaixo = (m3.distancia_baixo_cm > 0.0 && m3.distancia_baixo_cm <= 10.0);

  // 2. IDENTIFICAR A FASE DO DIA BASEADA NOS TEUS NOVOS LIMITES DE WATTS
  FaseDoDia novaFase;
  if (consumo_cidade_W < LIMITE_APAGAO_W) {
    novaFase = APAGAO;
  } else if (consumo_cidade_W <= LIMITE_NOITE_W) {
    novaFase = NOITE;  
  } else if (consumo_cidade_W <= LIMITE_MANHA_W) {
    novaFase = MANHA;  
  } else {
    novaFase = TARDE;  
  }

  if (novaFase != faseAtual) {
    Serial.printf("\n >>> MUDANÇA DE ESTADO: Consumo atual é %.3fW <<<\n", consumo_cidade_W);
    faseAtual = novaFase;
  }

  // 3. LÓGICA DE CONTROLO DIRETA
  int dutyMotor = 0;
  int dutyDissipacao = 0;
  bool motorSobe = true; // true = Sobe, false = Desce

  switch (faseAtual) {
    
    // ----------------------------------------------------------------------
    case APAGAO: // 0W
      // Ação: Motor desce para tentar gerar energia de arranque e repor a rede
      dutyDissipacao = 0;
      motorSobe = false; // Descer
      dutyMotor = 30;    // Velocidade de descida fixa de segurança
      
      if (maxBaixo) {
        dutyMotor = 0; // Proteção se chegar ao chão
      }
      break;

    // ----------------------------------------------------------------------
    case NOITE: // 0W até 0.04W
      // Ação: Motor SOBE. Bateria armazena o excesso.
      dutyDissipacao = 0; 
      motorSobe = true; // Subir
      
      // Velocidade de subida (quanto menos a cidade gasta, mais energia sobra para subir rápido)
      dutyMotor = map(balanco_rede_W * 100, 0, PRODUCAO_VIRTUAL_W * 100, 20, 100);

      if (maxCima) {
        dutyMotor = 0; // Proteção: Bateria cheia!
        // Queima o excesso na resistência porque já não pode subir mais
        dutyDissipacao = map(balanco_rede_W * 100, 0, PRODUCAO_VIRTUAL_W * 100, 0, 100);
      }
      break;

    // ----------------------------------------------------------------------
    case MANHA: // 0.04W até 0.22W
      // Ação: Motor MANTÉM (Para). Dissipação atua se houver excesso.
      dutyMotor = 0; // Mantém
      
      if (balanco_rede_W > 0) {
        dutyDissipacao = map(balanco_rede_W * 100, 0, PRODUCAO_VIRTUAL_W * 100, 0, 100);
      } else {
        dutyDissipacao = 0;
      }
      break;

    // ----------------------------------------------------------------------
    case TARDE: // > 0.22W (até 0.3W ou mais)
      // Ação: Motor DESCE para ajudar. Dissipação no máximo.
      dutyDissipacao = 100;
      motorSobe = false; // Descer
      
      // Mapeia a descida. Aos 0.22W desce devagar (30%), se chegar aos 0.30W desce no máximo (100%)
      dutyMotor = map(consumo_cidade_W * 1000, 220, 300, 30, 100); 
      
      if (maxBaixo) {
        dutyMotor = 0; // Proteção: Bateria vazia, não desce mais!
      }
      break;
  }

  // 4. APLICAÇÃO SEGURA DOS COMANDOS
  dutyMotor = constrain(dutyMotor, 0, 100);
  dutyDissipacao = constrain(dutyDissipacao, 0, 100);

  motorIBT2(dutyMotor, motorSobe);
  controlarDissipacao(dutyDissipacao);

  // Print de debug formatado para a tua apresentação (com 3 casas decimais para veres bem os mW)
  static unsigned long ultimoPrintLogs = 0;
  if (tempoAtual - ultimoPrintLogs > 2000) {
    Serial.printf("[REDE] Fase: %s | Consumo: %.3fW | Tensao: %.2fV | Motor: %d%% (%s) | Dissipacao: %d%%\n", 
                  nomesFases[faseAtual], consumo_cidade_W, tensao_cidade, dutyMotor, (motorSobe ? "Sobe" : "Desce"), dutyDissipacao);
    ultimoPrintLogs = tempoAtual;
  }

  delay(50); 
}