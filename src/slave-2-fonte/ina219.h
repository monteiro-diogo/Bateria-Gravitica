#ifndef INA219_H
#define INA219_H

// Estrutura para armazenar os dados de energia lidos do INA219
struct DadosEnergia {
  float tensao_V;
  float corrente_mA;
  float potencia_mW;
};

// Função para iniciar o sensor INA219
bool iniciarINA219();

// Função para ler os dados do sensor e preencher a estrutura DadosEnergia
DadosEnergia lerDadosINA219();

#endif