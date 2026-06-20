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

  static bool travaAnterior = false; // Guarda o estado anterior para não inundar o serial

  if (travaAtiva) {
    if (!travaAnterior) {
      Serial.println("\n[!!! ALERTA !!!] TRAVA ATIVADA! Objeto detetado a menos de 10cm. Motor bloqueado.");
      travaAnterior = true;
    }
    // Força o motor a parar independentemente do ciclo de tempo
    motorIBT2(0, true); 
    
    // Mantém o cronómetro atualizado para que quando a trava sair, 
    // ele recomece o ciclo de forma suave e não dê saltos.
    cronometroMotor = millis(); 
  } 
  else {
    if (travaAnterior) {
      Serial.println("\n[INFO] Caminho livre. Trava desativada, retomando operação...");
      travaAnterior = false;
      cronometroMotor = millis(); // Reinicia o ciclo ao destravar
    }

    unsigned long tempoAtual = millis();

    // MÁQUINA DE ESTADOS DO MOTOR (Só corre se a trava não estiver ativa)
    if (motorDeveRodar) {
      motorIBT2(100, true); 

      if (tempoAtual - cronometroMotor >= 20000) {
        motorDeveRodar = false;
        cronometroMotor = tempoAtual; 
        Serial.println("\n [MOTOR] 20s passaram. Parando o motor por 5s...\n");
      }
    } 
    else {
      motorIBT2(0, true); 

      if (tempoAtual - cronometroMotor >= 5000) {
        motorDeveRodar = true;
        cronometroMotor = tempoAtual; 
        Serial.println("\n [MOTOR] 5s passaram. Ativando o motor por 20s... \n");
      }
    }
  }
  
  delay(10);
}