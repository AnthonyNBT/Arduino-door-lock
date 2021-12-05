#include <RFID.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Keypad.h>

// Khai báo chân kết nối Arduino Mega với thiết bị
#define RFID522_SDA 10
#define RFID522_RESET 9
#define SERVO_PIN 12
#define BUZZER_PIN 11

const String RFID522_RESET_CODE = "cd67e8a9eb"; // Code thẻ RFID522 để reset hệ thống
const String RFID522_CODE = "6a9cc580b3";       // Code mật khẩu hệ thống bằng thẻ từ
const String KEYPAD_CODE = "123456";            // Code mật khẩu hệ thống bằng KeyPad

//Khai báo Keypad
const byte rows = 4;
const byte cols = 3;
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[rows] = {8, 7, 6, 5};
byte colPins[cols] = {4, 3, 2};

//Khai báo biến toàn cục
String keypad_Input_Code = "";
int numberOfDeny = 0;
String data = "non";

// Khai báo các thiết bị xử dụng trong hệ thống
LiquidCrystal_I2C myLCD(0x27,16,2);
Servo myServo;
RFID RC522(RFID522_SDA, RFID522_RESET); 
Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void setup() {
  Serial.begin(9600);
  
  //Khởi tạo RFID522
  SPI.begin(); 
  RC522.init();
  
  //Khởi tạo LCD
  startLCD();

  //Khởi tạo Servo
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  
  //Khởi tạo kết nối với ESP8266 Nodemcu
  Wire.begin(8);
  Wire.onRequest(requestEvent);
}

void loop() {
  if (RC522.isCard())                                         // Kiểm tra thiệt bị đầu vào có phải là thẻ từ hay không
  {
    String content = "";                                      // Biến lưu mã input thẻ từ
  
    RC522.readCardSerial();                                   // Đọc mã từ thẻ từ
    for(int i=0;i<5;i++)
    {
      content.concat(String(RC522.serNum[i], HEX));           // Lưu vào biến với hệ HEX
    }
    
    if (content == RFID522_RESET_CODE) {                      // Kiểm tra có phải thẻ Reset hệ thống hay không
      screenLCD(1, "Reset Access");
      myServo.write(180);
      delay(1000);
      screenLCD(1, "831 - 817 - 847");
      delay(2000);
      myServo.write(0);
      data = "14" + String(numberOfDeny);
      Serial.println(data);
      numberOfDeny = 0;
    } else {
      content = checkThreeTime(content);                      // Hàm kiểm tra số lần bị từ chối (Sai mật khẩu)
      handleType(1, content);                                 // In kết quả ra LCD và báo mã Code gửi sang ESP8266
    }
    keypad_Input_Code = "";                                   // Biến reset mật khẩu keypad
  }

  char keyPadInputKey = myKeypad.getKey();                    // Nhận diện phím người dùng ấn, nếu không ấn trả về false
  if (keyPadInputKey) {                                       // Kiểm tra người dùng có ấn phím không
    if (keyPadInputKey != '*' && keyPadInputKey != '#'){      // Kiểm tra phím người dùng ấn là phím số hay phím chức năng
      keypad_Input_Code.concat(String(keyPadInputKey));       // Nối kết quả các phím người dùng ấn
    }
    int passwordLength = keypad_Input_Code.length();          // Lấy chiều dài mất khẩu người dùng đã nhập
    if (passwordLength > 0 && passwordLength <= 16) {         // Kiểm tra chiều dài mất khẩu người dùng đã nhập vì LCD (16x2)
      String star = "";                                       // Chuỗi dâu * khi người dùng nhập mật khẩu keypadd
      for(int i = 0; i < passwordLength; i++) {
        star.concat("*");
      }
      screenLCD(1, star);                                     // In số '*' đại diện cho mật khẩu người dùng đã nhập ra LCD
    }
    if (keyPadInputKey == '#') {
      keypad_Input_Code = checkThreeTime(keypad_Input_Code);  // Hàm kiểm tra số lần bị từ chối (Sai mật khẩu)
      handleType(2,keypad_Input_Code);                        // In kết quả ra LCD và báo mã Code gửi sang ESP8266
      keypad_Input_Code = "";                                 // Biến reset mật khẩu keypad
    }
    if (keyPadInputKey == '*') {                              // Xóa nhập lại khi nhâp sai
      screenLCD(1,"831 - 817 - 847");                         // In MSSV thành viên ra màn hình khi dòng 2 trống
      keypad_Input_Code = "";                                 // Biến reset mật khẩu keypad
    }
  }
}
    
void startLCD(){                                              // Hàm khởi tạo LCD
  myLCD.init();
  myLCD.backlight();
  myLCD.setCursor(0,0);
  myLCD.print("   IoT Co ban   ");
  myLCD.setCursor(0,1);
  myLCD.print("831 - 817 - 847");
}

void screenLCD(int row, String message){                      // Hàm in dữ liệu chuỗi lên LCD
  myLCD.setCursor(0,row);
  myLCD.print("                ");
  myLCD.setCursor(0,row);
  myLCD.print(message);
}

void buzzer() {                                               // Hàm báo động cho Buzzer
  analogWrite(BUZZER_PIN, 250);
  delay(100);
  analogWrite(BUZZER_PIN, 225);
  delay(200);
  analogWrite(BUZZER_PIN, 200);
  delay(3000);
  analogWrite(BUZZER_PIN, 255);
}

void handleType(int type, String input) {                     // Hàm xử lý thông tin và gửi code về ESP8266
  /*
    Type: {1: RFID ; 2: KEYPAD}
    Status: {1: Access ; 2: Deny; 3: Warning, 4: Reset}
    NumberOfDeny: {}
  */
  
  String check = "";
  String l_type = "";
  
  switch (type){                                              // Switch chọn loại mật khẩu và chọn lại thiết bị
    case 1:
      check = RFID522_CODE;
      break;
    case 2:
      check = KEYPAD_CODE;
      break;
    default:
      check = "";
  }
  if (input == check) {                                       // Kiểm tra nếu mật khẩu đúng
    screenLCD(1, "Password Access");
    myServo.write(180);                                       // Servo mở khóa
    delay(1000);
    screenLCD(1, "831 - 817 - 847");
    delay(2000);
    myServo.write(0);                                         // Servo đóng khóa
    data = String(type * 100 + 1 * 10 + numberOfDeny);        // Code gửi về ESP8266
    Serial.println(data);                                     // Gửi code về ESP8266
    numberOfDeny = 0;                                         // Reset số lần sai khi mở khóa
  } else {                                                    // Nếu mật khẩu sai
    if (numberOfDeny < 3) {                                   // Nếu sai dưới 3 lần liên tục
      numberOfDeny += 1;                                      // Tăng số lần đã nhập sai
      screenLCD(1, "Password Deny");
      delay(1000);
      screenLCD(1, "831 - 817 - 847");
      data = String(type * 100 + 2 * 10 + numberOfDeny);      // Code gửi về ESP8266
      Serial.println(data);                                   // Gửi code về ESP8266
    }
  }//
  if (numberOfDeny >= 3){                                     // Nếu sai quá 3 lần, hệ thống bị khóa
    screenLCD(1, "System Locked");
    data = String(type * 100 + 3 * 10 + numberOfDeny);        // Code gửi về ESP8266
    Serial.println(data);                                     // Gửi code về ESP8266
    buzzer();                                                 // Còi báo động sẽ kêu
  }
}

String checkThreeTime(String input) {                         // Hàm kiểm tra xem người dùng đã nhập sai bao nhiêu lần
  if (numberOfDeny >= 3) {                                    // Nếu người dung đã nhập sai quá 3 lần thì không thể tiếp tục mở khóa hệ thống dù đúng mật khẩu
    return "";
  }
  return input;
}

void requestEvent() {                                         // Hàm gửi thông tin khởi tạo kết nối với ESP8266
 Wire.write(data.c_str());
 data = "non";
}
