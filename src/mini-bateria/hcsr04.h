#ifndef HCSR04_H
#define HCSR04_H

class HCSR04 {
  private:
    int trigPin;
    int echoPin;
    
  public:
    // Construtor que recebe os pinos específicos para este sensor
    HCSR04(int trig, int echo);
    
    // Inicia os pinos
    void iniciar();
    
    // Lê e devolve a distância
    float lerDistanciaCm();
};

#endif