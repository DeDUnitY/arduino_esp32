#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <HTTPClient.h>
const char* my_chat_id = "670366651";

#define WIFI_SSID "ded_unity"
#define WIFI_PASS "testtest1"
#define BOT_TOKEN "7263900065:AAHfgwyWy7Yn9uMxuitNDtEtcUMSXUPYmfs"
#include <FastBot.h>
FastBot bot(BOT_TOKEN);
WebServer server(80);

const char* svetAddress = "http://192.168.1.222/";

const int MONITOR_LED_PIN = 4; 
const int MONITOR_LED_COUNT = 56;
CRGB monitor_leds[MONITOR_LED_COUNT];

unsigned long bright_timer, off_timer,settings_timer,thisdelay;
String hex_color;

#pragma pack(push, 1)
struct Colors {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

int temp_number;
Colors monitor_color;
bool door_open = false;
bool put_color = false;
uint32_t eepromTimer = 0;
boolean eepromFlag = false;


bool touched = false;

void setup() {
  Serial.begin(9600);
  monitor_color.r = 0;
  monitor_color.g = 0;
  monitor_color.b = 0;

  EEPROM.begin(100);
  EEPROM.get(0, monitor_color);

  WiFi.setSleep(false);
  FastLED.addLeds<WS2812B, MONITOR_LED_PIN, GRB>(monitor_leds, MONITOR_LED_COUNT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.on("/", handleRoot);
  server.on("/color", handleColor);
  server.on("/door", handle_door);
  server.onNotFound(handleNotFound);
  server.begin();

  bot.attach(newMsg);
}


void checkEEPROM() {
  if (eepromFlag && (millis() - eepromTimer >= 15000) ) {
    eepromFlag = false;           // опустили флаг
    EEPROM.put(0, monitor_color);
    EEPROM.commit();
  }
}

void sendGETRequest() {
  HTTPClient http;
  http.begin(svetAddress);
  
  int httpCode = http.GET();
  
  http.end();
}

void handle_door(){
  if (server.hasArg("state")) {
    int state = server.arg("state").toInt();
    if (state == 1) door_open = false;
    else door_open = true;
  }
  server.send(200, "text/plain", "OK");
  if (door_open){
    for (int i=38; i < MONITOR_LED_COUNT; i++)
      monitor_leds[i].setRGB(255,0,0);
    FastLED.show();}
  else put_color = false;
}

void loop() {
  server.handleClient();
  if (!put_color){
    for (int i=0; i < MONITOR_LED_COUNT; i++)
      monitor_leds[i].setRGB(monitor_color.r,monitor_color.g,monitor_color.b);
    FastLED.show();
    put_color = true;}

  if (touchRead(27) < 25) {
    if (!touched) { // Если касание только началось
      touched = true; // Установить флаг касания
      sendGETRequest();
  }} else {
    touched = false; // Сбросить флаг, если касание прекратилось
  }
  bot.tick();
  checkEEPROM();
}

void newMsg(FB_msg& msg)
{
  if (msg.chatID != my_chat_id){
    Serial.println(msg.chatID);
    return;}
  if (msg.text == "/svet") {sendGETRequest();}
  else if (msg.text == "/white") { 
    monitor_color.r = 255;
    monitor_color.g = 255;
    monitor_color.b = 255;
    put_color = false;}
  else if (msg.text == "/off") { 
    monitor_color.r = 0;
    monitor_color.g = 0;
    monitor_color.b = 0;
    put_color = false;}
  else if (msg.text == "/red") { 
    monitor_color.r = 255;
    monitor_color.g = 0;
    monitor_color.b = 0;
    put_color = false;}
  else if (msg.text == "/green") {
   monitor_color.r = 0;
    monitor_color.g = 255;
    monitor_color.b = 0;
    put_color = false;};
}

void handleRoot() {
  String html = "<html><head><html lang=\"ru-RU\"><meta charset=\"UTF-8\"><style>body {background: linear-gradient(135deg, #001E25, 60%, black);font-family: Arial}";
  html += "form {color: #7D9093; font-size: 30px; margin-bottom: 15px;}";
  html += "h1 {margin-bottom: 3px; color: #7D9093;}";
  html += "input {background-color: #CFCFD1; margin-top: 10px;border-radius: 5px; border: 0px;width: 130px;height: 25px;font-size: 15px; letter-spacing: 0.01em;}";
  html += ".colorpicker{height: 26px; width: 80px;border-radius: 0px; }";
  html += ".slider {-webkit-appearance: none;width: 200px;height: 15px;border-radius: 10px;background: #a3a3a3;opacity: 0.7;}";
  html += ".slider::-webkit-slider-thumb {-webkit-appearance: none;width: 15px;height: 15px;border-radius: 50%;background: #00ffff;}";
  html += ".left {float: left;}";
  html += ".btn-group button {background-color: #04AA6D;border: 1px solid monitor_green;color: white;padding: 5px 5px;margin-right: 15px;width: 130px;display: block;font-size: 18px;}";
  html += "</style><title>PC Control</title></head><body>";
	html += "<h1>Monitor Led</h1>";
	html += "<form action=\"/color\">";
	html += "<input type=\"hidden\" name=\"for\" value=\"m\">";
	html += "<label for=\"color\">Color: </label>";
	html += "<input class=\"colorpicker\" type=\"color\" id=\"color\" name=\"color\" min=\"0\" max=\"255\" value=\"#"+String(monitor_color.r, HEX)+String(monitor_color.g, HEX)+String(monitor_color.b, HEX)+"\"><br>";
	html += "<input type=\"submit\" value=\"Send color\"><br></form></div>";
	html += "<div class=\"left\">";
  html += "</body></html>";
  server.send(200, "text/html", html);}

void handleColor() {
  hex_color = server.arg("color");
  temp_number = (int) strtol( &hex_color[1], NULL, 16);
  monitor_color.r = temp_number >> 16;
  monitor_color.g = temp_number >> 8 & 0xFF;
  monitor_color.b = temp_number & 0xFF;
  put_color = false;
  eepromFlag = true;                        // поднять флаг
  eepromTimer = millis(); 
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", " ");
}

void handleNotFound(){
  server.send(404, "text/html", "<h1>NOT FOUND</h1>");
  }
