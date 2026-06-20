#include "ibt2.h"
#include "telemetry.h"
#include "comms.h"
#include <Arduino.h>

/* Ligação do Driver IBT-2:
- RPWM: GPIO 25 (PWM Horário)
- LPWM: GPIO 26 (PWM Anti-horário)
- R_EN e L_EN: Ligados a 5V (ou a pinos GPIO em HIGH)
- Alimentação: 5V (VCC) e GND Comum com a ESP32
*/

#define RPWM_PIN 25 
#define LPWM_PIN 26
#define EN_PIN   27

// Definição dos canais PWM (exigidos na versão antiga do core ESP32)
#define RPWM_CANAL 0 
#define LPWM_CANAL 1 
#define FREQ_HZ 25000 
#define RESOLUTION_BITS 8 

// Definições de segurança
const int MAX_DUTY = 102; // Limite de 40% (102/255) para proteger motor 12V em fonte 24V
const float CORRENTE_MAXIMA_STALL_MA = 2000.0; // Limite de 2A para detectar bloqueio mecânico valor poderá ser ajustado 

bool iniciarIBT2() {
    // Configurar pino de habilitação
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, HIGH); // Liga a ponte H permanentemente

    ledcSetup(RPWM_CANAL, FREQ_HZ, RESOLUTION_BITS);
    ledcSetup(LPWM_CANAL, FREQ_HZ, RESOLUTION_BITS);
    ledcAttachPin(RPWM_PIN, RPWM_CANAL);
    ledcAttachPin(LPWM_PIN, LPWM_CANAL);
    
    return true;
}

void motorIBT2(int percentagem, bool sentidoHorario) {
    // --- TESTE DE ENTRADA DO MOTOR ---
    // Este print TEM de aparecer. Se não aparecer, o main.cpp não está a chamar esta função!
    static unsigned long ultimoAlertaChamada = 0;
    if (millis() - ultimoAlertaChamada >= 1000) {
        ultimoAlertaChamada = millis();
        Serial.printf("\n[SINAL] A funcao motorIBT2 esta a correr! Pedido: %d%%\n", percentagem);
    }

    // 1. Validação de entrada
    if (percentagem < 0) percentagem = 0;
    if (percentagem > 100) percentagem = 100;

    // Busca os dados diretamente do Mini 1 via comms.h
    struct_message m1 = getDadosMini1();
    
    // 2. PROTEÇÃO ANTI-STALL (BLOQUEIO MECÂNICO)
    if (m1.corrente_mA > CORRENTE_MAXIMA_STALL_MA) {
        // Se o código entrar aqui, avisa o porquê de ter travado
        if (millis() - ultimoAlertaChamada < 50) { 
            Serial.printf("!!! TRAVADO POR STALL !!! Corrente lida: %.2f mA\n", m1.corrente_mA);
        }
        ledcWrite(RPWM_CANAL, 0);
        ledcWrite(LPWM_CANAL, 0);
        return; // Bloqueia o resto do código
    }
    
    // 3. PROTEÇÃO POR TENSÃO (Regulação Dinâmica)
    float tensaoAtual = (m1.tensao_V < 1.0) ? 1.0 : m1.tensao_V;
    float fatorCorrecao = 7.0 / tensaoAtual;

    float percentagemCorrigida = (float)percentagem * fatorCorrecao;
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

// ========================================================================
// --- CÓDIGO DO SEGUNDO IBT-2 PARA DISSIPAÇÃO (NÃO MEXE NO MOTOR) ---
// ========================================================================

#define DISSIP_RPWM_PIN  32  // Novo pino PWM para a Dissipação (Ligar ao RPWM do 2º IBT-2)
#define DISSIP_CANAL     2   // Canal 2 (Os canais 0 e 1 já são do motor)

/* Ligação do 2º Driver IBT-2 (Dissipação):
- RPWM: GPIO 32
- LPWM: Ligado diretamente ao GND da ESP32 (não precisa de pino)
- R_EN e L_EN: Ligados juntos ao GPIO 33
- V+ / V- (Saída de Potência): Ligado à sua Resistência de Cimento de 10R
*/

bool iniciarDissipacao() {
    // Como o pino 27 já foi configurado como OUTPUT e HIGH no iniciarIBT2(),
    // aqui já não precisas de fazer mais nada com ele.
    
    // Configura apenas o novo canal PWM
    ledcSetup(DISSIP_CANAL, FREQ_HZ, RESOLUTION_BITS);
    ledcAttachPin(DISSIP_RPWM_PIN, DISSIP_CANAL);
    
    ledcWrite(DISSIP_CANAL, 0); 
    Serial.println("[DISSIPAÇÃO] Segundo IBT-2 configurado no PWM GPIO 32 (Enable partilhado no GPIO 27).");
    return true;
}

void controlarDissipacao(int percentagem) {
    if (percentagem < 0) percentagem = 0;
    if (percentagem > 100) percentagem = 100;

    // Mapeia 0-100% para 0-255 (Resolução de 8 bits)
    int duty = map(percentagem, 0, 100, 0, 255);
    ledcWrite(DISSIP_CANAL, duty);
}