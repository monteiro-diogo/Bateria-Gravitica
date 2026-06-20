#ifndef INA226_H
#define INA226_H

// Estrutura para armazenar os dados de energia lidos do INA226
struct DadosEnergia {
  float tensao_V;
  float corrente_mA;
  float potencia_mW;
};

// Função para iniciar o sensor INA226
bool iniciarINA226();

// Função para ler os dados do sensor e preencher a estrutura DadosEnergia
DadosEnergia lerDadosINA226();

#endif