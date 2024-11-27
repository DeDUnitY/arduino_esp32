#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const int switchLight = D2; // Пин, к которому подключен переключатель
const int Door = D1; // Пин, к которому подключен переключатель
const int outputPin = D8; // Пин, состояние которого нужно изменить

const char* ssid = "DSL-2750U";
const char* password = "MYpass-1";

const char* serverAddress = "http://192.168.1.209/door"; // Замените на ваш URL

WiFiClient main_client; // Создаем объект WiFiClient

ESP8266WebServer server(80);
bool state = true;
bool temp_state = false;
bool send_open = false;
bool send_close = false;
int switchState;
bool rele_change = true;

uint32_t eepromTimer = 0;
boolean eepromFlag = false;

void setup() {
  switchState = digitalRead(switchLight);

  EEPROM.begin(512);
  state = EEPROM.read(0); // читаем состояние из 0-го байта EEPROM
  EEPROM.end();

  pinMode(switchLight, INPUT);
  pinMode(Door, INPUT);
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  server.on("/", handleRoot);
  server.begin();
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

void sendPOSTRequest(int state) {
  HTTPClient http;
  http.begin(main_client, serverAddress); 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "state=" + String(state);
  int httpCode = http.POST(postData);
  http.end();
}

void checkEEPROM() {
  if (eepromFlag && (millis() - eepromTimer >= 15000) ) {
    eepromFlag = false;           // опустили флаг
    EEPROM.begin(512);
    EEPROM.write(0, state); // записываем состояние в 0-й байт EEPROM
    EEPROM.commit();
    EEPROM.end();
  }
}

void handleRoot() {
  state = !state;
  eepromFlag = true;                        // поднять флаг
  eepromTimer = millis();
  rele_change = true;
  server.send(200, "text/plain", "ok");}

void loop() {
  server.handleClient();
  checkEEPROM();

  if (rele_change){
    if (!state) {
      digitalWrite(outputPin, LOW);
    } else {
      digitalWrite(outputPin, HIGH);
      }
    delay(50);
  }

  if (switchState != digitalRead(switchLight)){
    switchState = digitalRead(switchLight);
    state = !state;
    eepromFlag = true;                        // поднять флаг
    eepromTimer = millis();
    rele_change = true;
  }

  if (digitalRead(Door) == HIGH and !send_open){
      sendPOSTRequest(0);
      send_open = true;
      send_close = false;
  }
  if (digitalRead(Door) == LOW and !send_close){
    sendPOSTRequest(1);
    send_close = true;
    send_open = false;
    }
}