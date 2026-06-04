#include "web_server.h"
#include "ina219.h"
#include "segredos.h"
#include "comms.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

WebServer server(80);

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// Função para iniciar o servidor web e configurar as rotas
void iniciarWebServer() {
  Serial.println("A ligar o Disco Rigido Interno (LittleFS)...");
    
  if (!LittleFS.begin(true)) {
    Serial.println("ERRO: Falha ao montar o LittleFS!");
    return;
  }

  // Para DEBUGGING (para veres o que tens gravado no disco da ESP32, se tiveres algo)
  Serial.println("\n--- CONTEUDO DO DISCO ---");
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  bool discoVazio = true;
  
  while(file){
      Serial.print("Encontrado: ");
      Serial.println(file.name());
      discoVazio = false;
      file = root.openNextFile();
  }
  if(discoVazio) {
      Serial.println("ALERTA: O DISCO ESTA COMPLETAMENTE VAZIO!");
  }
  Serial.println("-------------------------\n");

  Serial.println("[Master] A iniciar a Rede Wi-Fi");
  
  WiFi.mode(WIFI_AP_STA); // Força a placa a manter o modo Recetor ativo

  if (String(password).length() > 0) {
    WiFi.softAP(ssid, password, 1); // Canal 1
  } else {
    WiFi.softAP(ssid);
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Rede criada: "); Serial.println(ssid);
  Serial.print("Endereco do Painel: http://"); Serial.println(IP);
  
  // Configuração do servidor web para servir os ficheiros estáticos (HTML, CSS, JS) do LittleFS
  // 1. Rota explícita para a raiz: Força a abertura direta do ficheiro index.html
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(404, "text/plain", "ERRO interno: index.html inacessivel!");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  // 1. PRIMEIRO: Rotas específicas de API (os teus dados JSON)
  server.on("/dados", []() {
    DadosEnergia dados_master = lerDadosSensor();
    struct_message mini1 = getDadosMini1();
    struct_message mini2 = getDadosMini2();

    String json = "{";
    json += "\"v_master\":" + String(dados_master.tensao_V) + ",";
    json += "\"c_master\":" + String(dados_master.corrente_mA) + ",";
    json += "\"p_master\":" + String(dados_master.potencia_mW) + ",";
    json += "\"v_mini1\":" + String(mini1.tensao_V) + ",";
    json += "\"c_mini1\":" + String(mini1.corrente_mA) + ",";
    json += "\"p_mini1\":" + String(mini1.potencia_mW) + ",";
    json += "\"v_mini2\":" + String(mini2.tensao_V) + ",";
    json += "\"c_mini2\":" + String(mini2.corrente_mA) + ",";
    json += "\"p_mini2\":" + String(mini2.potencia_mW) + ",";
    json += "\"estado\":\"Sistema Online\"";
    json += "}";
    
    server.send(200, "application/json", json);
  });

  // 2. SEGUNDO: A rota explícita para o index.html (a solução da resposta anterior)
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(404, "text/plain", "ERRO interno: index.html inacessivel!");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  // 3. TERCEIRO: A rota genérica para servir CSS, JS e imagens
  server.serveStatic("/", LittleFS, "/");

  // 4. QUARTO: Salvaguarda global (Erro 404)
  server.onNotFound([]() {
    server.send(404, "text/plain", "ERRO: Ficheiro ou rota nao encontrada!");
  });

  server.begin();
  Serial.println("Servidor Web a correr!");
}

// Função para processar as requisições do servidor web (deve ser chamada no loop principal)
void processarWebServer() {
  server.handleClient(); 
}