#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>

const char* ssid = "Binh Thang";
const char* password = "K@$$123NBT090";

String serverName = "http://bb82-113-23-98-171.ngrok.io/getData";

int loop_char = 0;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);
  Wire.begin(D1, D2);
 
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  delay(500);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {  
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      Wire.requestFrom(8, 13);
      String data = "";
      while(Wire.available()){
        if (loop_char <3) {
          loop_char += 1;
          char c = Wire.read();
          data += c;
        }
        if (loop_char == 3) {
          loop_char = 0;
          break;
        }
      }
      if (!data.equals("non")){
        String serverPath = serverName + "?dataESP=" + data;
        
        http.begin(client, serverPath.c_str());
        
        int httpResponseCode = http.GET();
        
        if (httpResponseCode>0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
          Serial.println(payload);
        }
        else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        http.end();
      }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
 delay(1000);
}
