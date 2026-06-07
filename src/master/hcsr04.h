#ifndef HCSR04_H
#define HCSR04_H

// Função para iniciar os pinos do sensor ultrassónico
bool iniciarUltrassom();

// Função para ler o sensor e devolver a distância em centímetros
float lerDistanciaCm();

#endif