#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "dlink-48E3";  // Your WiFi SSID
const char* password = "afdda11084";  // Your WiFi Password

WebServer server(80);

const int ledPin = 5;  // LED connected to GPIO 5
const int pwmChannel = 0;
const int freq = 5000;
const int resolution = 8;

TaskHandle_t danceTask = NULL;
bool stopDance = false;

void handleCommand() {
  if (server.hasArg("cmd")) {
    String command = server.arg("cmd");
    server.sendHeader("Access-Control-Allow-Origin", "*");  // Enable CORS

    if (command == "on") {
      stopDance = true;
      ledcWrite(pwmChannel, 255);
      server.send(200, "text/plain", "LED ON");
    } 
    else if (command == "off") {
      stopDance = true;
      ledcWrite(pwmChannel, 0);
      server.send(200, "text/plain", "LED OFF");
    } 
    else if (command == "dance1") {
      startDance(dance1);
      server.send(200, "text/plain", "Dance 1 Started");
    }
    else if (command == "dance2") {
      startDance(dance2);
      server.send(200, "text/plain", "Dance 2 Started");
    }
    else if (command == "dance3") {
      startDance(dance3);
      server.send(200, "text/plain", "Dance 3 Started");
    }
    else if (command == "stop") {
      stopDance = true;
      server.send(200, "text/plain", "Dance Stopped");
    }
    else {
      server.send(400, "text/plain", "Invalid Command");
    }
  }
}

void handleBrightness() {
  if (server.hasArg("value")) {
    int brightness = server.arg("value").toInt();
    brightness = constrain(brightness, 0, 255);
    ledcWrite(pwmChannel, brightness);
    server.send(200, "text/plain", "Brightness Set: " + String(brightness));
  }
}

void startDance(void (*danceFunction)(void*)) {
  stopDance = true;
  delay(100);  // Allow previous task to exit
  stopDance = false;
  xTaskCreatePinnedToCore(danceFunction, "DanceTask", 2048, NULL, 1, &danceTask, 0);
}

void dance1(void *parameter) {
  while (!stopDance) {
    ledcWrite(pwmChannel, 255);
    delay(200);
    ledcWrite(pwmChannel, 0);
    delay(200);
  }
  vTaskDelete(NULL);
}

void dance2(void *parameter) {
  while (!stopDance) {
    for (int i = 0; i <= 255 && !stopDance; i += 25) {
      ledcWrite(pwmChannel, i);
      delay(50);
    }
    for (int i = 255; i >= 0 && !stopDance; i -= 25) {
      ledcWrite(pwmChannel, i);
      delay(50);
    }
  }
  vTaskDelete(NULL);
}

void dance3(void *parameter) {
  while (!stopDance) {
    ledcWrite(pwmChannel, 50);
    delay(100);
    ledcWrite(pwmChannel, 150);
    delay(100);
    ledcWrite(pwmChannel, 255);
    delay(100);
  }
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(ledPin, pwmChannel);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.println(WiFi.localIP());

  server.on("/control", handleCommand);
  server.on("/brightness", handleBrightness);
  server.begin();
}

void loop() {
  server.handleClient();
}
