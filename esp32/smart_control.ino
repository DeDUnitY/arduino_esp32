#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

// Replace with your network cmonitor_redentials
const char* ssid = "DSL-2750U";
const char* password = "MYpass-1";
#define serialRate 1000000   // скорость связи с ПК
// Create an instance of the WebServer
WebServer server(80);
unsigned long bright_timer, off_timer,settings_timer,thisdelay;
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;  // кодовое слово Ada для связи
// LED configuration
#define OFF_TIME 10 
const int MONITOR_LED_PIN = 16; 
const int MONITOR_LED_COUNT = 56;
const int ROOM_LED_PIN = 17; 
const int ROOM_LED_COUNT = 56;

// Variables for storing RGB values
CRGB monitor_leds[MONITOR_LED_COUNT];
CRGB room_leds[ROOM_LED_COUNT];
String hex_color;
int temp_number;
int monitor_red = 0;
int monitor_green = 0;
int monitor_blue = 0;
int room_red = 0;
int room_green = 0;
int room_blue = 0;
int brightness = 255;
int mode = 1;
void setup() {
  FastLED.addLeds<WS2812, MONITOR_LED_PIN, GRB>(monitor_leds, MONITOR_LED_COUNT);
  FastLED.addLeds<WS2812, ROOM_LED_PIN, GRB>(room_leds, ROOM_LED_COUNT);
  Serial.begin(serialRate);
  Serial.print("Ada\n");     // Связаться с компом
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }


  // Handle root URL
  server.on("/", handleRoot);

  // Handle form submission
  server.on("/color", handleColor);
  server.on("/brightness", handleBrightness);
  server.on("/mode1", handle_mode1);
  server.on("/mode2", handle_mode2);
  server.on("/mode3", handle_mode3);
  server.on("/mode4", handle_mode4);
  
  server.onNotFound(handleNotFound);
  // Start the server
  server.begin();
}

void loop() {
  // Handle client requests
  
  server.handleClient();
  if (mode ==4) screen_light();
  if (mode ==1)
    for (int i=0; i < MONITOR_LED_COUNT; i++)
      monitor_leds[i].setRGB(monitor_red,monitor_green,monitor_blue);
  FastLED.show();
  delay(100);
}
void check_connection() {
    if (millis() - off_timer > (OFF_TIME * 1000)) {
      mode = 1;
    }
}
void screen_light(){
  Serial.println(1);
  for (i = 0; i < sizeof prefix; ++i) {
  waitLoop: while (!Serial.available()) check_connection();;
      if (prefix[i] == Serial.read()) continue;
      i = 0;
      goto waitLoop;
    }
    
    while (!Serial.available()) check_connection();;
    hi = Serial.read();
    while (!Serial.available()) check_connection();;
    lo = Serial.read();
    while (!Serial.available()) check_connection();;
    chk = Serial.read();
    if (chk != (hi ^ lo ^ 0x55))
    {
      i = 0;
      goto waitLoop;
    }

    memset(monitor_leds, 0, MONITOR_LED_COUNT * sizeof(struct CRGB));
    for (int i = 0; i < MONITOR_LED_COUNT; i++) {
      byte r, g, b;
      // читаем данные для каждого цвета
      while (!Serial.available()) check_connection();
      r = Serial.read();
      while (!Serial.available()) check_connection();
      g = Serial.read();
      while (!Serial.available()) check_connection();
      b = Serial.read();
      monitor_leds[i].r = r;
      monitor_leds[i].g = g;
      monitor_leds[i].b = b;
    }
}

// Response handler for the root URL
void handleRoot() {
  // Create HTML page with sliders for RGB values
  String html = "<html><head><html lang=\"ru-RU\"><meta charset=\"UTF-8\"><style>body {background: linear-gradient(135deg, #001E25, 60%, black);font-family: Arial}";
  html += "form {color: #7D9093; font-size: 30px; margin-bottom: 15px;}";
  html += "h1 {margin-bottom: 3px; color: #7D9093;}";
  html += "input {background-color: #CFCFD1; margin-top: 10px;border-radius: 5px; border: 0px;width: 130px;height: 25px;font-size: 15px; letter-spacing: 0.01em;}";
  html += ".colorpicker{height: 26px; width: 80px;border-radius: 0px; }";
  html += ".slider {-webkit-appearance: none;width: 200px;height: 15px;border-radius: 10px;background: #a3a3a3;opacity: 0.7;}";
  html += ".slider::-webkit-slider-thumb {-webkit-appearance: none;width: 15px;height: 15px;border-radius: 50%;background: #00ffff;}";
  html += ".left {float: left;}";
  html += ".btn-group button {background-color: #04AA6D;border: 1px solid monitor_green;color: white;padding: 5px 5px;margin-right: 15px;width: 130px;display: block;font-size: 18px;}";
  html += ".mode"+String(mode)+" button {background-color: darkgray;}";
  html += "</style><title>PC Control</title></head>";
  html += "<body><h1>Monitor Led</h1>";
  html += "<div class=\"left\"><div class=\"btn-group\"><a href=\"/mode1\" class=\"mode1\" style=\"text-decoration: none;\"><button>Статичный<br>одноцветный</button></a>";
  html += "<a href=\"/mode2\" class=\"mode2\" style=\"text-decoration: none;\"><button>Динамичный<br>одноцветный</button></a>";
  html += "<a href=\"/mode3\" class=\"mode3\" style=\"text-decoration: none;\"><button>Динамичный<br>многоцветный</button></a>";
  html += "<a href=\"/mode4\" class=\"mode4\" style=\"text-decoration: none;\"><button>Расширение<br>экрана</button></a></div></div>";
  html += "<div class=\"left\">";
	html += "<h1>Monitor Led</h1>";
	html += "<form action=\"/brightness\" >";
		html += "<input type=\"hidden\" name=\"for\" value=\"m\">";
		html += "<label for=\"brightness\">Brightness: </label>";
		html += "<input type=\"range\" class=\"slider\" id=\"brightness\" name=\"brightness\" min=\"0\" max=\"255\" value=\"255\"><br>";
		html += "<input type=\"submit\" value=\"Send brightness\"></form><form action=\"/color\">";
		html += "<input type=\"hidden\" name=\"for\" value=\"m\">";
		html += "<label for=\"color\">Color: </label>";
		html += "<input class=\"colorpicker\" type=\"color\" id=\"color\" name=\"color\" min=\"0\" max=\"255\" value=\"\"><br>";
		html += "<input type=\"submit\" value=\"Send color\"><br></form></div>";
	html += "<div class=\"left\">";
	html += "<h1>Room Led</h1>";
	html += "<form action=\"/brightness\">";
		html += "<input type=\"hidden\" name=\"for\" value=\"r\">";
		html += "<label for=\"brightness\">Brightness: </label>";
		html += "<input type=\"range\" class=\"slider\" id=\"brightness\" name=\"brightness\" min=\"0\" max=\"255\" value=\"255\"><br>";
		html += "<h1>";
		html += "<input type=\"submit\" value=\"Send brightness\">";
	html += "</form>";
	html += "<form action=\"/color\">";
		html += "<input type=\"hidden\" name=\"for\" value=\"r\">";
		html += "<label for=\"color\">Color: </label>";
		html += "<input class=\"colorpicker\" type=\"color\" id=\"color\" name=\"color\" min=\"0\" max=\"255\" value=\"\"><br>";
		html += "<input type=\"submit\" value=\"Send color\"><br></form>";
  html += "</body></html>";
  
  // Send response to the client
  server.send(200, "text/html", html);}
// Response handler for form submission
void handleColor() {
  // Get the RGB values from the request parameters
  if (mode ==1){
    hex_color = server.arg("color");
    temp_number = (int) strtol( &hex_color[1], NULL, 16);
    if (server.arg("for")=="m"){
      monitor_red = temp_number >> 16;
      monitor_green = temp_number >> 8 & 0xFF;
      monitor_blue = temp_number & 0xFF;
      // Set the LED strip color
      for (int i=0; i < MONITOR_LED_COUNT; i++)
        monitor_leds[i].setRGB(monitor_red,monitor_green,monitor_blue);}
    else{
      room_red = temp_number >> 16;
      room_green = temp_number >> 8 & 0xFF;
      room_blue = temp_number & 0xFF;
      for (int i=0; i < ROOM_LED_COUNT; i++)
        room_leds[i].setRGB(room_red,room_green,room_blue);}}
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", " ");
  }
  

void handle_mode1(){
    mode = 1;
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", " ");}
void handle_mode2(){
    mode = 2;
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", " ");}
void handle_mode3(){
    mode = 3;
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", " ");}
void handle_mode4(){
    mode = 4;
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", " ");}
void handleBrightness(){
  brightness = server.arg("brightness").toInt();
  //pixels.setBrightness(brightness);
  //pixels.show();
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", " ");}
void handleNotFound(){
  server.send(404, "text/html", "<h1>NOT FOUND</h1>");
  }
