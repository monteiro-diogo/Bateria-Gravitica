#include "ina226.h"
#include <Wire.h>
#include <INA226.h>

// A biblioteca do Rob Tillaart exige o endereço I2C
INA226 ina226(0x44); 

/* Ligação do INA226:
- SDA: GPIO 21
- SCL: GPIO 22
- Alimentação: 3V3 e GND comum com a ESP32
- V+: Lado da fonte
- V-: Lado da carga (para medir a corrente que passa pela carga em série)
- VBUS: Positivo da fonte (para medir a tensão do barramento) = V+
*/
// Exemplo: Se quiseres usar o Pino 4 e o Pino 5
#define I2C_SDA_PIN 21   // Liga o fio SDA do INA226 neste pino
#define I2C_SCL_PIN 22   // Liga o fio SCL do INA226 neste pino

bool iniciarINA226() {
  Serial.println("\n--- A INICIAR O SENSOR INA226 ---");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // --- DEBUG: SCANNER I2C ---
  Serial.println("A procurar dispositivos I2C nos pinos " + String(I2C_SDA_PIN) + " e " + String(I2C_SCL_PIN) + "...");
  byte count = 0;
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    // Se o dispositivo responder, o endereçamento foi bem-sucedido e substituímos na linha 6
    if (Wire.endTransmission() == 0) {
      Serial.print("Encontrado sensor no endereco: 0x");
      Serial.println(i, HEX);
      count++;
    }
  }
  
  if (count == 0) {
    Serial.println("[INA226] ERRO FISICO: Nenhum sensor encontrado! Verificar os fios e a alimentacao.");
    while(1) { delay(100); } // Pára a placa aqui para evitar loop de erros
  }
  Serial.println("---------------------------------");
  // --- FIM DO SCANNER I2C ---

  // Iniciar a Biblioteca
  if (!ina226.begin()) { 
    Serial.println("[INA226] ERRO: Biblioteca INA226 falhou no begin()!"); 
    while(1) { delay(10); } // Pára a placa
  }
  
  // --- CONFIGURAÇÃO CRÍTICA DO NOVO SHUNT R009 ---
  // Definimos o teto para 8 Amperes por segurança, com o shunt de 0.009 Ohms
  // No máximo colocamos 9A para evitar o risco de queimar o sensor, mesmo que o sistema possa chegar a 10A em picos.
  ina226.setMaxCurrentShunt(8.0, 0.009); 
  
  Serial.println("[INA226] Sensor iniciado com sucesso.");
  return true;
}

DadosEnergia lerDadosINA226() {
  DadosEnergia dados;
  
  // Lê os valores da biblioteca
  dados.tensao_V = ina226.getBusVoltage();
  
  float corrente_A = ina226.getCurrent(); 
  dados.corrente_mA = abs(corrente_A * 1000.0); 
  
  dados.potencia_mW = dados.tensao_V * dados.corrente_mA; // Potência em mW

  // Filtro de ruído (Podes descomentar quando o sistema estiver no local final)
  if (dados.tensao_V < 1.0) {
    dados.tensao_V = 0.0;
    dados.corrente_mA = 0.0;
    dados.potencia_mW = 0.0;
  }
  
  return dados;
}