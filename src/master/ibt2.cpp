#include "ibt2.h"

#if __has_include("ina226.h")
    #include "ina226.h"
#else
    struct DadosEnergia {
        float tensao_V;
        float corrente_mA; 
        float potencia_mW; 
    };
    static DadosEnergia lerDadosINA226() {
        return {12.0f, 0.0f, 0.0f}; 
    }
#endif

#include <Arduino.h>

/* Ligação do Driver IBT-2:
- RPWM: GPIO 25 (PWM Horário)
- LPWM: GPIO 26 (PWM Anti-horário)
- R_EN e L_EN: Ligados a 5V (ou a pinos GPIO em HIGH)
- Alimentação: 5V (VCC) e GND Comum com a ESP32
*/

#define RPWM_PIN 25 
#define LPWM_PIN 26

// Definição dos canais PWM (exigidos na versão antiga do core ESP32)
#define RPWM_CANAL 0 
#define LPWM_CANAL 1 
#define FREQ_HZ 25000 
#define RESOLUTION_BITS 8 

// Definições de segurança
const int MAX_DUTY = 102; // Limite de 40% (102/255) para proteger motor 12V em fonte 24V
const float CORRENTE_MAXIMA_STALL_MA = 2000.0; // Limite de 2A para detectar bloqueio mecânico valor poderá ser ajustado 

bool iniciarIBT2() {
    ledcSetup(RPWM_CANAL, FREQ_HZ, RESOLUTION_BITS);
    ledcSetup(LPWM_CANAL, FREQ_HZ, RESOLUTION_BITS);
    ledcAttachPin(RPWM_PIN, RPWM_CANAL);
    ledcAttachPin(LPWM_PIN, LPWM_CANAL);
    return true;
}

void motorIBT2(int percentagem, bool sentidoHorario) {
    // 1. Validação de entrada
    if (percentagem < 0) percentagem = 0;
    if (percentagem > 100) percentagem = 100;

    DadosEnergia d = lerDadosINA226();
    
    // 2. PROTEÇÃO ANTI-STALL (BLOQUEIO MECÂNICO)
    // Se a corrente subir acima do limite, o motor está a esforçar-se sem rodar.
    if (d.corrente_mA > CORRENTE_MAXIMA_STALL_MA) {
        Serial.println("!!! ALERTA: Motor Bloqueado (Stall) - Desligando !!!");
        ledcWrite(RPWM_CANAL, 0);
        ledcWrite(LPWM_CANAL, 0);
        return; // Interrompe a execução para evitar queimar bobinas
    }
    
    // 3. PROTEÇÃO POR TENSÃO (Regulação Dinâmica)
    // Evita divisão por zero e mantém a "tensão média" nos 11V ideais
    float tensaoAtual = (d.tensao_V < 1.0) ? 1.0 : d.tensao_V;
    float fatorCorrecao = 11.0 / tensaoAtual;

    // Aplica o fator sobre a percentagem (ex: 24V -> fator 0.45 -> reduz potência)
    float percentagemCorrigida = (float)percentagem * fatorCorrecao;

    // Garante que não ultrapassamos o limite absoluto de 40% (102)
    if (percentagemCorrigida > 100.0) percentagemCorrigida = 100.0;

    // 4. Mapeamento final para o PWM
    int duty = map((int)percentagemCorrigida, 0, 100, 0, MAX_DUTY);

    // 5. Comando do Driver H-Bridge
    if (sentidoHorario) {
        ledcWrite(RPWM_CANAL, duty);
        ledcWrite(LPWM_CANAL, 0);
    } else {
        ledcWrite(RPWM_CANAL, 0);
        ledcWrite(LPWM_CANAL, duty);
    }
}

void testarIBT2() {
    motorIBT2(30, true);
    delay(5000); 
}