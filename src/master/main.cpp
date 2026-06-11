#include <Arduino.h>
#include "web_server.h"
#include "comms.h"
#include "config.h"
#include "telemetry.h"
#include "hardware.h"
#include "sg90.h"

// Configurações iniciais do sistema
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(DELAY_ARRANQUE_MS);

  iniciarWebServer(); // Inicia a rede AP
  initComms();        // Inicia o recetor ESP-NOW 
  iniciarHardware(); // Inicia os sensores locais 

  Serial.println("Sistema online!");
}

// Loop principal
void loop() {
  processarWebServer(); // Mantém o servidor web a funcionar (responder a pedidos HTTP)
  imprimirTelemetria(); // Imprime os dados do Master e dos Minis na consola série a cada segundo

  return;
}