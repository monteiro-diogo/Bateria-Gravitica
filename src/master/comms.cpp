#include "comms.h"
#include <WiFi.h>
#include <esp_now.h>

// Variáveis para guardar em memória a última leitura de cada Mini
struct_message ultimaLeituraMini1 = {1, 0.0, 0.0, 0.0};
struct_message ultimaLeituraMini2 = {2, 0.0, 0.0, 0.0};

// Função Callback: O que fazer quando uma mensagem chega do "ar"
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct_message dadosRecebidos;
  memcpy(&dadosRecebidos, incomingData, sizeof(dadosRecebidos));
  
  //  Atualiza a última leitura da Mini correspondente com os dados recebidos
  if (dadosRecebidos.id == 1) {
    ultimaLeituraMini1 = dadosRecebidos;
  } else if (dadosRecebidos.id == 2) {
    ultimaLeituraMini2 = dadosRecebidos;
  }
}

// Função para iniciar a comunicação ESP-NOW
void initComms() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERRO: Falha ao inicializar o ESP-NOW no Master!");
    return;
  }
  
  // Diz ao ESP32 para usar a nossa função OnDataRecv quando chegarem dados
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("[Master] ESP-NOW Iniciado. A escuta...");
}

// Funções "Getter" para o web_server.cpp ir buscar os dados de forma segura
struct_message getDadosMini1() { return ultimaLeituraMini1; }
struct_message getDadosMini2() { return ultimaLeituraMini2; }