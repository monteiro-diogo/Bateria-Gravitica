#include <Arduino.h>
#include "web_server.h"
#include "comms.h"
#include "config.h"
#include "telemetry.h"
#include "hardware.h"
#include "sg90.h"
#include "ibt2.h" 

// Variáveis para o controlo de tempo do motor (Máquina de Estados)
unsigned long cronometroMotor = 0;
bool motorDeveRodar = true;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) { delay(10); } // Garante que a consola série abre antes dos prints
  
  delay(DELAY_ARRANQUE_MS);

  // Inicializações (Ordem correta e sem duplicações)
  iniciarWebServer(); 
  initComms(); 
  iniciarHardware(); // <--- Esta função já chama o iniciarIBT2() lá dentro!

  Serial.println("Sistema online!");
  cronometroMotor = millis(); // Inicia o contador de tempo
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); // Vai cuspir os dados no terminal baseado no intervalo configurado

  unsigned long tempoAtual = millis();

  // MÁQUINA DE ESTADOS DO MOTOR
  if (motorDeveRodar) {
    // Se está no estado LIGADO, mantém o motor a 100%
    motorIBT2(100, true); 

    // Se já passaram 20 segundos (20000 ms), muda o estado para DESLIGADO
    if (tempoAtual - cronometroMotor >= 20000) {
      motorDeveRodar = false;
      cronometroMotor = tempoAtual; // Reinicia o cronómetro para o próximo estado
      Serial.println("\n>>>> [CICLO MOTOR] 20s passaram. Parando o motor por 5s... <<<<\n");
    }
  } 
  else {
    // Se está no estado DESLIGADO, força o motor a 0%
    motorIBT2(0, true); 

    // Se já passaram 5 segundos (5000 ms), volta ao estado LIGADO
    if (tempoAtual - cronometroMotor >= 5000) {
      motorDeveRodar = true;
      cronometroMotor = tempoAtual; // Reinicia o cronómetro para o próximo estado
      Serial.println("\n>>>> [CICLO MOTOR] 5s passaram. Ativando o motor por 20s... <<<<\n");
    }
  }
  
  delay(10); // Estabilidade do processador
}