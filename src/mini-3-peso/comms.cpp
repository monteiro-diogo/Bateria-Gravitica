#include "comms.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "segredos.h" // Onde temos o array: uint8_t MAC_ADDRESS[] = {...} do master;

struct_message meusDados;
esp_now_peer_info_t peerInfo;

// Callback que nos avisa se o pacote chegou efetivamente ao Master
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Envio para o Master: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("SUCESSO (Master recebeu)");
  } else {
    Serial.println("FALHA (Master nao encontrado)");
  }
}

void initComms() {
  // Colocar o Mini em modo Station (necessário para o ESP-NOW neste caso)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  // Iniciar a rede ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO: Falha ao inicializar ESP-NOW no Mini!");
    return;
  }

  // Registar a função que verifica se o envio teve sucesso
  esp_now_register_send_cb(OnDataSent);

  // Limpar a estrutura do peer antes de preencher
  memset(&peerInfo, 0, sizeof(peerInfo));

  // Registar o Master como destino (Peer)
  memcpy(peerInfo.peer_addr, MAC_ADDRESS, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("[ESP-NOW] ERRO: Falha ao registar o Master (Peer)!");
    return;
  }
  
  Serial.println("[ESP-NOW] ESP-NOW Iniciado no Mini. Master emparelhado.");
}

void sendData(int id, float dist_cima, float dist_baixo) {
  // Prepara os dados na estrutura atualizada
  meusDados.id = id;
  meusDados.distancia_cima_cm = dist_cima;
  meusDados.distancia_baixo_cm = dist_baixo;

  // Dispara os dados para o ar na direção do MAC do Master
  esp_err_t result = esp_now_send(MAC_ADDRESS, (uint8_t *) &meusDados, sizeof(meusDados));
   
  if (result != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO: Ocorreu um problema a enviar os dados via ESP-NOW.");
  }
}