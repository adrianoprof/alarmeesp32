#include "AdafruitIO_WiFi.h"
#include "NewPing.h"

#define BUZZER_PIN 27
#define LED_ALARME 13
#define BOTAO_FISICO 26
#define TRIG_PIN 12
#define ECHO_PIN 14

//Configuação do ultrassonico
#define MAX_DISTANCE 100
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Configurações Wifi e Adafruit
#define IO_USERNAME "..."
#define IO_KEY "..."

#define WIFI_SSID "..."
#define WIFI_PASS "..."

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

//Definição dos Feeds
AdafruitIO_Feed *botaoalarme = io.feed("botaoalarme");
AdafruitIO_Feed *distanciaultrassom = io.feed("distanciaultrassom");

// Variáveis de controle
bool alarmeAtivo = false;
unsigned int distancia = 0;
const int LIMITE_DISTANCIA = 15;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ALARME, OUTPUT);
  pinMode(BOTAO_FISICO, INPUT);

  Serial.begin(115200);

  while (!Serial)
    ;

  io.connect();

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("Adafruit Conectado!");

  //Vincula a função callback ao feed
  botaoalarme->onMessage(handleAlarme);

  Serial.println("Solicitando o estado inicial do alarme: ");
  botaoalarme->get();

  delay(1000);
}

void loop() {
  io.run();

  //Leitura do botão físico
  if (digitalRead(BOTAO_FISICO) == 1) {
    delay(300);  // debounce simples
    alarmeAtivo = !alarmeAtivo;

    botaoalarme->save(String(alarmeAtivo ? "true" : "false"));

    Serial.println(alarmeAtivo ? F("Alarme ARMADO pelo botao fisico.") : F("Alarme DESARMADO pelo botao fisico."));
  }

  distancia = sonar.ping_cm();
  Serial.print("Distancia lida: ");
  Serial.println(distancia);
  Serial.println(" cm");

  if (distancia != 0) {
    //só envia distancias válidas
    distanciaultrassom -> save(distancia);
  }

  //ativação ou desativação do alarme
  if (alarmeAtivo && distancia > 0 && distancia < LIMITE_DISTANCIA) {
    ativarAlerta();
  } else {
    desligarAlerta();
  }

  delay(3000);  // intervalo ideal para a Adafruit
}
