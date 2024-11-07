#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "GFX4d.h"

// Configuração Wi-Fi e MQTT
const char* ssid = "iPhone de Pedro";
const char* password = "birulinha";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Inicialização do Display e Cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);
GFX4d gfx = GFX4d();

// Dimensões e posição do quadrado (exemplo: centralizado na tela)
const int squareX = 50;
const int squareY = 70;
const int squareWidth = 150;
const int squareHeight = 150;

// Dimensões máximas para posX e posY
const int maxPosX = 320;
const int maxPosY = 240;

void drawSquare() {
  // Desenha o contorno do quadrado
  gfx.Rectangle(squareX, squareY, squareX + squareWidth, squareY + squareHeight, WHITE);
}

void drawPoint(int posX, int posY) {
  // Limpar ponto anterior (apagar área dentro do quadrado)
  gfx.RectangleFilled(squareX + 1, squareY + 1, squareX + squareWidth - 1, squareY + squareHeight - 1, BLACK);

  // Mapear posX e posY para as dimensões do quadrado
  int mappedX = map(posX, 0, maxPosX, squareX, squareX + squareWidth);
  int mappedY = map(posY, 0, maxPosY, squareY, squareY + squareHeight);

  // Desenhar o ponto atualizado
  gfx.CircleFilled(mappedX, mappedY, 3, RED); // Raio 3 define o tamanho do ponto
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("Erro ao analisar JSON: ");
    Serial.println(error.c_str());
    return;
  }

  int tgt_id = doc["tgt_id"];
  const char* tgt_type = doc["tgt_type"];
  int posX = doc["tgt_position"]["posX"];
  int posY = doc["tgt_position"]["posY"];

  drawPoint(posX, posY);
  
  gfx.TextSize(2); // tamanho da fonte
  gfx.TextColor(WHITE, BLACK);

  // exibir os dados
  gfx.MoveTo(0,10);
  gfx.print("ID:");
  gfx.println(tgt_id);
  gfx.print("Tipo:");
  gfx.println(tgt_type);
  gfx.print("Pos X:");
  gfx.print(posX);
  gfx.print(" Pos Y:");
  gfx.print(posY);
  gfx.println("\n\n");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP8266_Client")) {
      Serial.println("conectado!");
      client.subscribe("sinal/acompanhamento");
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi conectado.");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  gfx.begin();
  gfx.Cls();
  gfx.BacklightOn(true);
  gfx.Orientation(PORTRAIT);

  drawSquare(); // Desenhar o quadrado uma vez no setup
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
