
#include <WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define NUM_LEDS 60
#define DATA_PIN 6
#define LED_TYPE    WS2812B

CRGB leds[NUM_LEDS];
StaticJsonDocument<250> jsonDocument;

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
int status = WL_IDLE_STATUS;
int WIFI_LOGIN_ATTEMPTS = 3;

WiFiServer server(80);

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
    FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
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

void getBody() {
    if (server.hasArg("plain") == false) {
        
    }
    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);
}

void postLed() {
    
    Serial.println("POST /led");
    getBody();

    Serial.println(jsonDocument['led']);
    Serial.println(jsonDocument['r']);
    Serial.println(jsonDocument['g']);
    Serial.println(jsonDocument['b']);
    
    int led = (int)jsonDocument['led'];
    int r = (int)jsonDocument['r'];
    int g = (int)jsonDocument['g'];
    int b = (int)jsonDocument['b'];

    set_led_color(led, r, g, b);
    
}

void getLed() {
    Serial.println("GET /led");
}

void postLeds() {
    Serial.println("POST /leds");
    getBody();
    JsonArray array = jsonDocument['leds'].as<JsonArray>();
    for(JsonVariant led : array) {
        Serial.println(led['led']);
        Serial.println(led['r']);
        Serial.println(led['g']);
        Serial.println(led['b']);
    }
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
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new web client");
    bool currentLineIsBlank = true;

    while (client.connected()) {

      if (client.available()) {

        char c = client.read();

        Serial.write(c);

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply

        if (c == '\n' && currentLineIsBlank) {

          // send a standard http response header

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
        
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
  delay(10); // this speeds up the simulation
}
