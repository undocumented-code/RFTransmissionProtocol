#include <elapsedMillis.h>
#include <ESP8266WiFi.h>

elapsedMillis elapsedTime;

const uint16_t port = 6379;
const char* host = "redis-host";
const char* ssid = "wifi-network-name";
const char* key = "wifipassword";

const char* aggrigatorId = "aggrigator1";

unsigned long lasttime = millis();
unsigned int data = 0;
unsigned int lastdata = 0;
unsigned long slidingWindow = 0x00000000;
unsigned long captureWindow = 0x00000000;

void setup() {
  pinMode(D5, INPUT);
  Serial.begin(115200);
  delay(10);

  WiFi.begin(ssid, key);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(500);
}

void loop() {
  data = digitalRead(D5);

  if (data && !lastdata) { //rising edge
    lasttime = millis();
    lastdata = data;
  }
  if (!data && lastdata) { //falling edge
    lastdata = data;
    unsigned long delta = millis() - lasttime;
    if (delta < 40 || delta > 120) {
      return;
    }
    slidingWindow = slidingWindow << 1;
    if (delta > 30 && delta < 70) {} //is zero
    if (delta > 80 && delta < 120) {
      slidingWindow |= 1UL;
    }

    unsigned char topFrame = (slidingWindow >> 24) & 0xFF;
    unsigned char bottomFrame = (slidingWindow & 0x0F);
    if (topFrame == 0xBD && bottomFrame == 0x0B) {
      Serial.print("CAUGHT IT: ");
      captureWindow = slidingWindow;
      unsigned int content = (captureWindow >> 8) & 0xFFFF;
      unsigned char checksum = (captureWindow >> 4) & 0x0F;
      Serial.print(content, HEX);
      Serial.print(", ");
      Serial.print(checksum, HEX);
      Serial.print(", ");
      unsigned char calcchecksum = 0x00;
      for (char i = 0; i < 4; i++) calcchecksum ^= (((unsigned char) (content >> (4 * i))) & 0x0F);
      if (checksum == calcchecksum) {
        Serial.println("is valid");
        Serial.println("submitting change to redis");

        char value = content & 0x01;
        short sensorId = content >> 1;

        Serial.print("connecting to ");
        Serial.println(host);
        WiFiClient client;

        if (!client.connect(host, port)) {
          Serial.println("connection failed");
          return;
        }

        // This will send the request to the server
        char currentCommand[100];
        sprintf(currentCommand, "set %s_%d %d\n", aggrigatorId, sensorId, value);
        client.print(currentCommand);

        String line = client.readStringUntil('\r');        
        Serial.println(line);
        Serial.println("closing connection");
        client.stop();
      }
      else Serial.println("is invalid");
    }
  }
}
