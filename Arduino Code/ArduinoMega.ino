#include <RFID.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Keypad.h>

// Define Pin
#define RFID522_SDA 10
#define RFID522_RESET 9
#define SERVO_PIN 12
#define BUZZER_PIN 11

const String RFID522_RESET_CODE = "cd67e8a9eb";
const String RFID522_CODE = "6a9cc580b3";
const String KEYPAD_CODE = "123456";
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
String keypad_Input_Code = "";
int numberOfDeny = 0;
String data = "non";

LiquidCrystal_I2C myLCD(0x27,16,2);
Servo myServo;
RFID RC522(RFID522_SDA, RFID522_RESET); 
Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void setup() {
  Serial.begin(9600);
  
  //RC522 Start
  SPI.begin(); 
  RC522.init();
  
  //LCD Start
  startLCD();

  //Servo Start
  myServo.attach(SERVO_PIN);
  myServo.write(0);

  Wire.begin(8);
  Wire.onRequest(requestEvent);
}

void loop() {
  if (RC522.isCard())
  {
    String content = "";
  
    RC522.readCardSerial();
    for(int i=0;i<5;i++)
    {
      content.concat(String(RC522.serNum[i], HEX));
    }
    
    if (content == RFID522_RESET_CODE) {
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
    content = checkThreeTime(content);
    handleType(1, content);
  }
  keypad_Input_Code = "";
  }

  char keyPadInputKey = myKeypad.getKey();
  if (keyPadInputKey) {
    if (keyPadInputKey != '*' && keyPadInputKey != '#'){
      keypad_Input_Code.concat(String(keyPadInputKey));
    }
    int passwordLength = keypad_Input_Code.length();
    if (passwordLength > 0 && passwordLength <= 16) {
      String star = "";
      for(int i = 0; i < passwordLength; i++) {
        star.concat("*");
      }
      screenLCD(1, star);      
    }
    if (keyPadInputKey == '#') {
      keypad_Input_Code = checkThreeTime(keypad_Input_Code);
      handleType(2,keypad_Input_Code);
      keypad_Input_Code = "";  
    }
    if (keyPadInputKey == '*') {
      screenLCD(1,"831 - 817 - 847");
      keypad_Input_Code = "";  
    }
  }
}
    
void startLCD(){
  myLCD.init();
  myLCD.backlight();
  myLCD.setCursor(0,0);
  myLCD.print("   IoT Co ban   ");
  myLCD.setCursor(0,1);
  myLCD.print("831 - 817 - 847");
}

void screenLCD(int row, String message){
  myLCD.setCursor(0,row);
  myLCD.print("                ");
  myLCD.setCursor(0,row);
  myLCD.print(message);
}

void buzzer() {
  analogWrite(BUZZER_PIN, 250);
  delay(100);
  analogWrite(BUZZER_PIN, 225);
  delay(200);
  analogWrite(BUZZER_PIN, 200);
  delay(3000);
  analogWrite(BUZZER_PIN, 255);
}

void handleType(int type, String input) {
  /*
    Type: {1: RFID ; 2: KEYPAD}
    Status: {1: Access ; 2: Deny; 3: Warning, 4: Reset}
    NumberOfDeny: {}
  */
  
  String check = "";
  String l_type = "";
  
  switch (type){
    case 1:
      check = RFID522_CODE;
      break;
    case 2:
      check = KEYPAD_CODE;
      break;
    default:
      check = "";
  }
  if (input == check) {
    screenLCD(1, "Password Access");
    myServo.write(180);
    delay(1000);
    screenLCD(1, "831 - 817 - 847");
    delay(2000);
    myServo.write(0);
    data = String(type * 100 + 1 * 10 + numberOfDeny);
    Serial.println(data);
    numberOfDeny = 0;
  } else {
    if (numberOfDeny < 3) {
      numberOfDeny += 1;
      screenLCD(1, "Password Deny");
      delay(1000);
      screenLCD(1, "831 - 817 - 847");
      data = String(type * 100 + 2 * 10 + numberOfDeny);
      Serial.println(data);
    }
  }//
  if (numberOfDeny >= 3){
    screenLCD(1, "System Locked");
    data = String(type * 100 + 3 * 10 + numberOfDeny);
    Serial.println(data);
    buzzer();
  }
}

String checkThreeTime(String input) {
  if (numberOfDeny >= 3) {
    return "";
  }
  return input;
}

void requestEvent() {
 Wire.write(data.c_str());
 data = "non";
}
