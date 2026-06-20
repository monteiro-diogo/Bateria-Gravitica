#ifndef IBT2_H
#define IBT2_H

bool iniciarIBT2();

void motorIBT2(int percentagem, bool sentidoHorario);


// --- FUNÇÕES ADICIONADAS PARA O SEGUNDO IBT-2 (DISSIPAÇÃO) ---
bool iniciarDissipacao();
void controlarDissipacao(int percentagem);

#endif