#include "ina219.h"
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Função para iniciar o sensor INA219
bool iniciarSensor() { 
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (!ina219.begin()) {
    return false;
  }
  return true;
}

// Função para ler os dados do sensor e preencher a estrutura DadosEnergia
DadosEnergia lerDadosSensor() {
  float busvoltage = 0;
  float current_mA = 0;
  float power_mW = 0;
  
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  // No caso de corrente negativa
  if (current_mA < 0) {
    current_mA = std::fabs(current_mA); 
  }

  // Para evitar ruido
  if (busvoltage < 1.0) {
    busvoltage = 0.0;
    current_mA = 0.0;
    power_mW = 0.0;
  }

  // Preencher o pacote
  DadosEnergia dados_master;
  dados_master.tensao_V = busvoltage;
  dados_master.corrente_mA = current_mA;
  dados_master.potencia_mW = power_mW;

  return dados_master;
}