#include "ina219.h"
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

/* Ligação do INA219:
- SDA: GPIO 21
- SCL: GPIO 22
- Alimentação: 3v3 e GND comum com a ESP32
- V+: Lado da fonte
- V-: Lado da carga (para medir a corrente que passa pela carga em série)
*/

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Função para iniciar o sensor INA219
bool iniciarINA219() { 
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!ina219.begin()) {
    return false;
  }
 
  return true;
}

// Função para ler os dados do sensor e preencher a estrutura DadosEnergia
DadosEnergia lerDadosINA219() {
  float busvoltage = 0;
  float current_mA = 0;
  float power_mW = 0;
  
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();

  if (current_mA < 0) {
    current_mA = abs(current_mA); 
  }

  power_mW = busvoltage * current_mA;

  // Evitar ruido
  if (busvoltage < 1.2) {
    busvoltage = 0.0;
    current_mA = 0.0;
    power_mW = 0.0;
  }

  // Preencher struct DadosEnergia
  DadosEnergia dados_master;
  dados_master.tensao_V = busvoltage;
  dados_master.corrente_mA = current_mA;
  dados_master.potencia_mW = power_mW;

  return dados_master;
}