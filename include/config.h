#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- COMUNICAÇÃO SÉRIE ---
const unsigned long SERIAL_BAUD_RATE = 115200;

// --- TEMPOS (Milissegundos) ---
const unsigned long DELAY_ARRANQUE_MS = 1000;
const unsigned long INTERVALO_TELEMETRIA_MS = 1000;
const unsigned long TIMEOUT_MINIS_MS = INTERVALO_TELEMETRIA_MS * 3; // Se não recebermos dados de um Mini há mais de 3 vezes o intervalo, consideramos offline

#endif