#include <Arduino.h>
#include "web_server.h"
#include "comms.h"
#include "config.h"
#include "telemetry.h"
#include "hardware.h"
#include "sg90.h"
#include "ibt2.h" 
#include "protocol.h" 

// Variáveis para o controlo de tempo do motor (Máquina de Estados)
unsigned long cronometroMotor = 0;
bool motorDeveRodar = true;

// Variável para controlar o tempo de impressão das distâncias no terminal
unsigned long ultimoPrintDistancias = 0;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) { delay(10); } 
  
  delay(DELAY_ARRANQUE_MS);

  iniciarWebServer(); 
  initComms(); 
  iniciarHardware(); 
  
  // INICIAR O SEGUNDO IBT-2 DE DISSIPAÇÃO
  iniciarDissipacao(); 

  Serial.println("Sistema online!");
  cronometroMotor = millis();
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); 

  // Travagem por altura
  struct_message m3 = getDadosMini3(); 

  bool maxCima = (m3.distancia_cima_cm > 0.0 && m3.distancia_cima_cm <= 10.0);
  bool maxBaixo = (m3.distancia_baixo_cm > 0.0 && m3.distancia_baixo_cm <= 10.0);
  bool travaAtiva = (maxCima || maxBaixo);

  static bool travaAnterior = false; 

  if (travaAtiva) {
    if (!travaAnterior) {
      Serial.println("\n[!!! ALERTA !!!] TRAVA ATIVADA! Motor bloqueado. Ativando dissipação de segurança.");
      travaAnterior = true;
    }
    // Se travou por segurança, desliga o motor e ativa a dissipação a 50%
    motorIBT2(0, true); 
    controlarDissipacao(50); 
    
    cronometroMotor = millis(); 
  } 
  else {
    if (travaAnterior) {
      Serial.println("\n[INFO] Caminho livre. Trava desativada, retomando operação...");
      travaAnterior = false;
      cronometroMotor = millis(); 
    }

    unsigned long tempoAtual = millis();

    // MÁQUINA DE ESTADOS (Só corre se a trava não estiver ativa)
    if (motorDeveRodar) {
      motorIBT2(100, true); 
      controlarDissipacao(0); // Motor ligado -> Dissipação DESLIGADA (0%)

      if (tempoAtual - cronometroMotor >= 20000) { // 20 Segundos Ligado
        motorDeveRodar = false;
        cronometroMotor = tempoAtual; 
        Serial.println("\n [MOTOR] 20s passaram. Parando o motor e LIGANDO DISSIPAÇÃO (50%)...\n");
      }
    } 
    else {
      motorIBT2(0, true); 
      controlarDissipacao(50); // Motor desligado -> Dissipação LIGADA (50% Duty Cycle)

      if (tempoAtual - cronometroMotor >= 10000) { // !!! AUMENTADO PARA 10 SEGUNDOS DESLIGADO !!!
        motorDeveRodar = true;
        cronometroMotor = tempoAtual; 
        Serial.println("\n [MOTOR] 10s passaram. Ativando o motor e DESLIGANDO DISSIPAÇÃO... \n");
      }
    }
  }
  
  delay(10);
}