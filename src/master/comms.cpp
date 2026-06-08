#include "comms.h"
#include <WiFi.h>
#include <esp_now.h>

// Variáveis para guardar em memória a última leitura de cada Mini
struct_message ultimaLeituraMini1 = {1, 0.0, 0.0, 0.0};
struct_message ultimaLeituraMini2 = {2, 0.0, 0.0, 0.0};

volatile unsigned long ultimoTempoMini1 = 0;
volatile unsigned long ultimoTempoMini2 = 0;

String estadoMini1 = "ONLINE";
String estadoMini2 = "ONLINE";

// Função Callback: O que fazer quando uma mensagem chega do "ar"
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct_message dadosRecebidos;
  
  // Segurança para evitar estouro de memória se o tamanho não coincidir
  if (len == sizeof(struct_message)) {
    memcpy(&dadosRecebidos, incomingData, sizeof(dadosRecebidos));
    
    // Debug para confirmar na consola do Master que o rádio captou o sinal
    Serial.print("[ESP-NOW] ESP-C3 Mini a comunicar: ");
    Serial.println(dadosRecebidos.id);

    if (dadosRecebidos.id == 1) {
      ultimaLeituraMini1 = dadosRecebidos;
      ultimoTempoMini1 = millis();
    } else if (dadosRecebidos.id == 2) {
      ultimaLeituraMini2 = dadosRecebidos;
      ultimoTempoMini2 = millis();
    }
  } else {
    Serial.print("[ESP-NOW] ERRO: Tamanho de pacote invalido recebido: ");
    Serial.println(len);
  }
}

// Função para iniciar a comunicação ESP-NOW
void initComms() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO: Falha ao inicializar o ESP-NOW no Master!");
    return;
  }
  
  // Diz ao ESP32 para usar a nossa função OnDataRecv quando chegarem dados
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("[ESP-NOW] Iniciado com sucesso.");
}

void atualizarEstados() {
  estadoMini1 = (millis() - ultimoTempoMini1 > TIMEOUT_MINIS_MS) ? "OFFLINE" : "ONLINE";
  estadoMini2 = (millis() - ultimoTempoMini2 > TIMEOUT_MINIS_MS) ? "OFFLINE" : "ONLINE";
}

bool isMini1Online() { atualizarEstados(); return (estadoMini1 == "ONLINE"); }
bool isMini2Online() { atualizarEstados(); return (estadoMini2 == "ONLINE"); }

struct_message getDadosMini1() { 
  if (!isMini1Online()) { ultimaLeituraMini1 = {1, 0.0, 0.0, 0.0}; }
  return ultimaLeituraMini1; 
}

struct_message getDadosMini2() { 
  if (!isMini2Online()) { ultimaLeituraMini2 = {2, 0.0, 0.0, 0.0}; }
  return ultimaLeituraMini2; 
}