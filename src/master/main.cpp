#include <Arduino.h>
#include "web_server.h"
#include "comms.h"
#include "config.h"
#include "telemetry.h"
#include "hardware.h"
#include "sg90.h"
#include "ibt2.h" // Incluindo o seu driver

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(DELAY_ARRANQUE_MS);

  // Inicializações
  iniciarIBT2();      // <--- Adicionado: Inicia os pinos e canais PWM do IBT2
  iniciarWebServer(); 
  initComms(); 
  iniciarHardware(); 

  Serial.println("Sistema online!");
}

void loop() {
  processarWebServer(); 
  imprimirTelemetria(); 

  // Exemplo de uso:
  // Aqui você deve decidir quando o motor deve rodar.
  // Por exemplo, usando um valor vindo de uma variável global ou comando:
   motorIBT2(50, true); 
  
  delay(10); // Pequena pausa para estabilidade do processador
}