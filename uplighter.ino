#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define NUM_LEDS 16
#define DATA_PIN 5
#define LED_TYPE    WS2812B

CRGB leds[NUM_LEDS];
StaticJsonDocument<1536> jsonDocument;

char ssid[] = "TNCAP373511";
char pass[] = "AqNGZHA3yFt93GJL";
int status = WL_IDLE_STATUS;
int WIFI_LOGIN_ATTEMPTS = 3;
byte mac[6];

WebServer server(80);

void print_var(char * placeholder, char * message, char * var){
  int buffer_len = 100;
  char buffer[buffer_len];
  char outputbuffer[buffer_len];
  strcpy(buffer, message);
  strcat(buffer, " ");
  strcat(buffer, placeholder);
  snprintf(outputbuffer, buffer_len, buffer, var);
  Serial.println(outputbuffer);
}

void print_mac(){
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}

void bytes_to_hex(byte * byte_array, int array_length, char * output){
  const char hex_chars[]= "0123456789ABCDEF";
  for(int i=array_length; i >= 0; i--){
    output[i] = hex_chars[(byte_array[i>>1] >> ((1 - (i&1)) << 2)) & 0xF];
  }
  output[array_length] = 0; //Terminate the string
}

void setup_led_matrix(){
    Serial.println("Setting up LED Matrix 4x4");
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void connect_wifi() {
  int attempts = 0;
  Serial.println("Connecting to wifi");
  while (status != WL_CONNECTED && (attempts <= WIFI_LOGIN_ATTEMPTS)) {
    print_var("%d", "Attempt number", (char * )attempts);
    status = WiFi.begin(ssid, pass);
    attempts ++;
    delay(10000);
  }
  if(status != WL_CONNECTED) {
    print_var("%s", "Failed to connect to wifi", ssid);
  } else {
    print_var("%s", "Connected to wifi", ssid);
    WiFi.macAddress(mac);
    Serial.println(WiFi.localIP());
    print_mac();
  }
}

void start_server() {
  Serial.println("Starting web server");
  server.begin();
  Serial.println("Server started");
}

void set_led_color(int led_id, int r, int g, int b){
    leds[led_id].red = r;
    leds[led_id].green = g;
    leds[led_id].blue = b;
    FastLED.show();
}

int getBody() {
    if (server.hasArg("plain") == false) {
        Serial.println("HTML Body expected but not found");
    }
    String body = server.arg("plain");
    Serial.println("Body received:");
    Serial.println(body);
    DeserializationError error = deserializeJson(jsonDocument, body);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return false;
    }
    return true;
}

void postLed() {
    Serial.println("POST /led");
    int id = server.arg(0).toInt();
    if(getBody() == true) {
      int r = jsonDocument["r"].as<int>();
      int g = jsonDocument["g"].as<int>();
      int b = jsonDocument["b"].as<int>();

      print_var("%d", "r: ", (char *)r);
      print_var("%d", "g: ", (char *)g);
      print_var("%d", "b: ", (char *)b);
      
      set_led_color(id, r, g, b);
      jsonDocument.clear();
      server.send(200, "application/json", "{}");
      
      return;
    }
    server.send(400, "application/json", "{}");
    return;
}

void getLed() {
    Serial.println("GET /led");
    int id = server.arg(0).toInt();
    String output = "";
    jsonDocument["r"] = leds[id].r;
    jsonDocument["g"] = leds[id].g;
    jsonDocument["b"] = leds[id].b;
    serializeJsonPretty(jsonDocument, output); 
    jsonDocument.clear();
    server.send(200, "application/json", output);
    return;
}

void postLeds() {
    Serial.println("POST /leds");
    int id = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    if(getBody() == true) {
      
      for(int i=0; i < NUM_LEDS; i++) {
          Serial.println(jsonDocument["leds"][i]["id"].as<const char *>());
          Serial.println(jsonDocument["leds"][i]["r"].as<const char *>());
          Serial.println(jsonDocument["leds"][i]["g"].as<const char *>());
          Serial.println(jsonDocument["leds"][i]["b"].as<const char *>());
          id = jsonDocument["leds"][i]["id"].as<int>();
          r = jsonDocument["leds"][i]["r"].as<int>();
          g = jsonDocument["leds"][i]["g"].as<int>();
          b = jsonDocument["leds"][i]["b"].as<int>();
          set_led_color(id, r, g, b);
          id = 0;
          r = 0;
          g = 0;
          b = 0;
      }
      server.send(200, "application/json", "{}");
      jsonDocument.clear();
      return;
    }
    server.send(400, "application/json", "{}");
    return;
}

void getLeds() {
    Serial.println("GET /leds");
}

void getIdent() {
  Serial.println("GET _ident");
  String output = "";
  int n = sizeof mac << 1;
  char mac_string[n + 1];
  bytes_to_hex(mac, n, mac_string);
  
  jsonDocument["ident"] = mac_string;
  serializeJsonPretty(jsonDocument, output);
  server.send(200, "application/json", output);
  jsonDocument.clear();
}

void setup_server_routes() {
    Serial.println("Setting up server routes...");
    server.on("/_ident", HTTP_GET, getIdent);
    server.on("/led", HTTP_POST, postLed);
    server.on("/led", HTTP_GET, getLed);
    server.on("/leds", HTTP_POST, postLeds);
    server.on("/leds", HTTP_GET, getLeds);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Uplighter starting...");
  setup_led_matrix();
  connect_wifi();
  setup_server_routes();
  start_server();
}

void loop() {
  server.handleClient();
  delay(10);
}
