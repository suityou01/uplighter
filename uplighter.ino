
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define NUM_LEDS 60
#define DATA_PIN 5
#define LED_TYPE    WS2812B

CRGB leds[NUM_LEDS];
StaticJsonDocument<250> jsonDocument;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
int status = WL_IDLE_STATUS;
int WIFI_LOGIN_ATTEMPTS = 3;

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
}

int getBody() {
    if (server.hasArg("plain") == false) {
        
    }
    String body = server.arg("plain");
    DeserializationError error = deserializeJson(jsonDocument, body);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    return false;
  }
}

void postLed() {
    
    Serial.println("POST /led");
    if(getBody() == true) {
      Serial.println(jsonDocument['led'].as<const char*>());
      Serial.println(jsonDocument['r'].as<const char*>());
      Serial.println(jsonDocument['g'].as<const char*>());
      Serial.println(jsonDocument['b'].as<const char*>());
    
      int led = jsonDocument['led'].as<int>();
      int r = jsonDocument['r'].as<int>();
      int g = jsonDocument['g'].as<int>();
      int b = jsonDocument['b'].as<int>();

      set_led_color(led, r, g, b);
    }
}

void getLed() {
    Serial.println("GET /led");
}

void postLeds() {
    Serial.println("POST /leds");
    getBody();
    JsonArray array = jsonDocument['leds'].as<JsonArray>();
    /*
    for(JsonVariant led : array) {
        Serial.println((char *)led['led']);
        Serial.println((char *)led['r']);
        Serial.println((char *)led['g']);
        Serial.println((char *)led['b']);
    }
    */
}

void getLeds() {
    Serial.println("GET /leds");
}

void setup_server_routes() {
    Serial.println("Setting up server routes...");
    server.on("/led", HTTP_POST, postLed);
    server.on("/led", HTTP_GET, getLed);
    server.on("/leds", HTTP_POST, postLeds);
    server.on("/leds", HTTP_GET, getLeds);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Uplighter starting...");
  setup_led_matrix();
  connect_wifi();
  setup_server_routes();
  start_server();
}

void loop() {
  server.handleClient();
  delay(10); // this speeds up the simulation
}