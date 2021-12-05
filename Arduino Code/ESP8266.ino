#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>

const char* ssid = "Binh Thang";                                    // Khai báo SSID Wifi
const char* password = "K@$$123NBT090";                             // Khai báo mật khẩu Wifi

String serverName = "http://bb82-113-23-98-171.ngrok.io/getData";   // Link trang web của Web Server

int loop_char = 0;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);                                             // Kết nối Serial đến band 115200
  Wire.begin(D1, D2);                                               // Kết nối port đến Arduino
 
  WiFi.begin(ssid, password);                                       // Kết nối Wifi
  Serial.println("Connecting");
  delay(500);
  while(WiFi.status() != WL_CONNECTED) {                            // Kiểm tra ESP8266 đã kết nối đến Wifi chưa
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {  
  if ((millis() - lastTime) > timerDelay) {                         // Kiểm tra thời gian giữa 2 lần gửi thông tin về Web Server
    if(WiFi.status()== WL_CONNECTED){                               // Kiểm tra Wifi đã kết nối hay chưa
      WiFiClient client;                                            // Khởi tạo Wifi Client
      HTTPClient http;                                              // Khởi tạo HTTP Client

      Wire.requestFrom(8, 13);                                      // Lấy dữ liệu từ Arduino
      String data = "";                                             // Biến chứa dữ liệu từ Arduino
      while(Wire.available()){                                      // Kiểm tra kết nối
        if (loop_char <3) {                                         // Lưu dữ liệu từ Arduino vào biến
          loop_char += 1;
          char c = Wire.read();
          data += c;
        }
        if (loop_char == 3) {
          loop_char = 0;
          break;
        }
      }
      if (!data.equals("non")){                                     // Kiểm tra xem dữ liệu có rổng không
        String serverPath = serverName + "?dataESP=" + data;        // URL gửi thông tin về Web Serer
        
        http.begin(client, serverPath.c_str());                     // Bắt đầu khởi tạo đường truyền thông tin với HTTP
        
        int httpResponseCode = http.GET();                          // Gửi đi bằng phương thức GET
        
        if (httpResponseCode>0) {                                   // Kiểm ra kết quả gửi đi được hay không
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
          Serial.println(payload);
        }
        else {                                                      // Sai thì in lỗi
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        http.end();
      }
    }
    else {                                                          // Thông báo ngắt kết nối Wifi
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
 delay(1000);
}
