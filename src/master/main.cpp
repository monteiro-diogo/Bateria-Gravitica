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
// CONFIGURAÇÕES E LIMITES
// ==============================================================================
const float LIMITE_CARGA_mW = 1000.0; 
const float LIMITE_ESTAVEL_mW = 1200.0; 

enum EstadoRede { REDE_LEVE, REDE_ESTAVEL, REDE_SOBRECARGA };
EstadoRede estadoAtual = REDE_LEVE;
const char* nomesEstados[] = {"LEVE", "ESTAVEL", "PICO"};

// Variáveis de Ciclo
unsigned long tempoInicioPico = 0;
bool emCicloDePico = false;
bool quedaConcluida = false;

// Configuração do Servo (Curso de 90º - Invertido)
// 150 = Travado, 60 = Solto (Diferença de 90º)
const int ANGULO_TRAVADO = 180; 
const int ANGULO_SOLTO = 80;    
const unsigned long TEMPO_ALIVIO_TRAVA_MS = 4000;
int ultimoAnguloServo = -1;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(DELAY_ARRANQUE_MS);

  iniciarWebServer(); 
  initComms(); 
  iniciarHardware(); 
  iniciarDissipacao();
  iniciarSG90();

  moverSG90(ANGULO_TRAVADO); 
  ultimoAnguloServo = ANGULO_TRAVADO;
  Serial.println("[SISTEMA] Smart Grid Online.");
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); 

  unsigned long tempoAtual = millis();
  struct_message m1 = getDadosMini1(); 
  struct_message m3 = getDadosMini3(); 
  
  float consumo_cidade_mW = m1.potencia_mW;
  bool maxCima = (m3.distancia_cima_cm > 0.0 && m3.distancia_cima_cm <= 20.0);
  bool maxBaixo = (m3.distancia_baixo_cm > 0.0 && m3.distancia_baixo_cm <= 20.0);

  // Identificar Estado
  EstadoRede novoEstado;
  if (consumo_cidade_mW < LIMITE_CARGA_mW) novoEstado = REDE_LEVE;
  else if (consumo_cidade_mW <= LIMITE_ESTAVEL_mW) novoEstado = REDE_ESTAVEL;
  else novoEstado = REDE_SOBRECARGA;

  if (novoEstado != estadoAtual) estadoAtual = novoEstado;

  // Lógica de Controlo
  int dutyMotor = 0;
  int dutyDissipacao = 0;
  bool motorSobe = true;
  bool abrirTrava = false; 

  switch (estadoAtual) {
    case REDE_LEVE:
      emCicloDePico = false;
      quedaConcluida = false; 
      motorSobe = true;
      abrirTrava = false; 
      dutyMotor = map(constrain(consumo_cidade_mW, 0, LIMITE_CARGA_mW), 0, LIMITE_CARGA_mW, 100, 50);
      if (maxCima) dutyMotor = 0;
      break;

    case REDE_ESTAVEL:
      emCicloDePico = false;
      quedaConcluida = false;
      dutyMotor = 0;
      abrirTrava = false; 
      break;

    case REDE_SOBRECARGA:
      if (!emCicloDePico) {
        emCicloDePico = true;
        quedaConcluida = false; 
        tempoInicioPico = tempoAtual;
      }

      unsigned long tempoDecorrido = tempoAtual - tempoInicioPico;

      if (tempoDecorrido <= TEMPO_ALIVIO_TRAVA_MS) {
        motorSobe = true; dutyMotor = 40; abrirTrava = true;
      } else {
        unsigned long tempoCiclo = tempoDecorrido - TEMPO_ALIVIO_TRAVA_MS;
        if (tempoCiclo > 105000) { tempoInicioPico = tempoAtual; quedaConcluida = false; }

        if (tempoCiclo <= 30000) dutyDissipacao = map(tempoCiclo, 0, 30000, 0, 100);
        else if (tempoCiclo <= 75000) dutyDissipacao = 100;
        else dutyDissipacao = map(tempoCiclo, 75000, 105000, 100, 0);

        if (!quedaConcluida && tempoCiclo <= 75000) {
          motorSobe = false; abrirTrava = true;
          dutyMotor = map(dutyDissipacao, 0, 100, 30, 100); 
          if (maxBaixo) { dutyMotor = 0; abrirTrava = false; quedaConcluida = true; }
        } else {
          dutyMotor = 0; abrirTrava = false;
        }
      }
      break;
  }

  // Execução
  motorIBT2(constrain(dutyMotor, 0, 100), motorSobe);
  controlarDissipacao(constrain(dutyDissipacao, 0, 100));
  
  int anguloDesejado = abrirTrava ? ANGULO_SOLTO : ANGULO_TRAVADO;
  if (anguloDesejado != ultimoAnguloServo) {
    moverSG90(anguloDesejado);
    ultimoAnguloServo = anguloDesejado;
  }

  delay(50); 
}