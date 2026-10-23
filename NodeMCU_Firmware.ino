/*
  NodeMCU (ESP8266) firmware for the "NodeMCU Control" app
  Car (L298N) + 4 relays + 1 servo, controlled over Wi-Fi HTTP.

  Board : NodeMCU 1.0 (ESP-12E Module)   Libraries: built into the ESP8266 core
  Pins  : Relays  D1, D2, D5, D6
          Servo   D7
          Motor   IN1=D0  IN2=D3  IN3=D4  IN4=D8  (jumper ENA/ENB to 5V on the L298N)
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ---- EDIT THESE ----
const char* WIFI_SSID = "YourWiFi";
const char* WIFI_PASS = "YourPassword";
const char* DEVICE_NAME = "NodeMCU-1";
const bool  RELAY_ACTIVE_LOW = true;   // most relay modules switch ON with LOW
// --------------------

const int relayPins[4] = {D1, D2, D5, D6};
const int IN1 = D0, IN2 = D3, IN3 = D4, IN4 = D8;
const int SERVO_PIN = D7;

ESP8266WebServer server(80);
Servo servo;
bool relayState[4] = {false, false, false, false};
int servoAngle = 90;
unsigned long lastCarCmd = 0;
bool carMoving = false;

void cors() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Private-Network", "true");
}
void sendJson(const String& s) { cors(); server.send(200, "application/json", s); }

void writeRelay(int i, bool on) {
  relayState[i] = on;
  digitalWrite(relayPins[i], (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void drive(char d) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  carMoving = (d != 'S');
  switch (d) {
    case 'F': digitalWrite(IN1, HIGH); digitalWrite(IN3, HIGH); break;
    case 'B': digitalWrite(IN2, HIGH); digitalWrite(IN4, HIGH); break;
    case 'L': digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); break;
    case 'R': digitalWrite(IN1, HIGH); digitalWrite(IN4, HIGH); break;
    default: break;
  }
  lastCarCmd = millis();
}

String statusJson() {
  String s = "{\"relays\":[";
  for (int i = 0; i < 4; i++) { s += relayState[i] ? "1" : "0"; if (i < 3) s += ","; }
  s += "],\"servo\":" + String(servoAngle) + "}";
  return s;
}

void setup() {
  for (int i = 0; i < 4; i++) { pinMode(relayPins[i], OUTPUT); writeRelay(i, false); }
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  drive('S');
  servo.attach(SERVO_PIN); servo.write(servoAngle);

  // Station mode (home Wi-Fi); falls back to its own hotspot if it can't connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 12000) delay(250);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("NodeMCU-Control", "12345678");   // hotspot IP: 192.168.4.1
  }

  server.on("/ping", []() {
    String ip = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    sendJson("{\"device\":\"nodemcu\",\"name\":\"" + String(DEVICE_NAME) + "\",\"mac\":\"" + WiFi.macAddress() + "\",\"ip\":\"" + ip + "\"}");
  });
  server.on("/status", []() { sendJson(statusJson()); });
  server.on("/relay", []() {            // /relay?n=1..4&s=0|1
    int n = server.arg("n").toInt(), s = server.arg("s").toInt();
    if (n >= 1 && n <= 4) writeRelay(n - 1, s == 1);
    sendJson(statusJson());
  });
  server.on("/servo", []() {            // /servo?a=0..180
    servoAngle = constrain(server.arg("a").toInt(), 0, 180);
    servo.write(servoAngle);
    sendJson(statusJson());
  });
  server.on("/car", []() {              // /car?d=F|B|L|R|S
    String d = server.arg("d");
    drive(d.length() ? d[0] : 'S');
    sendJson(statusJson());
  });
  server.begin();
}

void loop() {
  server.handleClient();
  // Safety: stop the car if the app stops sending commands (signal lost / finger lifted)
  if (carMoving && millis() - lastCarCmd > 1200) drive('S');
}
