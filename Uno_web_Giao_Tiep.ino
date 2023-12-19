#include <SoftwareSerial.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
//động cơ
#include <Stepper.h>
int button = 13;
int led4 = 12;
int led5 = 48;
int led6 = 46;
const byte coi = 53;
const int stepsPerRevolution = 1024; // 4 Steps Sequence
Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11); // Cài đặt động cơ bước và cấu hình chân cắm.

int potentiometer = A0;
int Val_Analog,MotorSpeed;
//LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
const byte RX = 2;
const byte TX = 3;

int cambien = A1;
SoftwareSerial mySerial(RX, TX); // Initialize SoftwareSerial

String inputString = ""; // A string to hold incoming data
bool stringComplete = false; // A flag to check if the string is complete

int gas = A2;
int as = 44;
#define DHTTYPE DHT22 // DHT 11
#define DHTPIN 4 // Pin to which the DHT11 is connected
#define LED1PIN 5
#define LED2PIN 6
#define LED3PIN 7

DHT dht(DHTPIN, DHTTYPE);
long last = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  
  // Initialize serial communication:
  Serial.begin(9600);
  while (!Serial) {
    // Wait for the serial port to be ready
  }

  mySerial.begin(9600); // Initialize SoftwareSerial
  while (!mySerial) {
    // Wait for the SoftwareSerial port to be ready
  }

  dht.begin(); 
  pinMode(gas, INPUT);
  pinMode(as, INPUT);
  
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(coi, OUTPUT);
  pinMode(cambien, INPUT);
  pinMode(LED1PIN, OUTPUT);
  digitalWrite(LED1PIN, LOW);
  pinMode(LED2PIN, OUTPUT);
  digitalWrite(LED2PIN, LOW);
  pinMode(LED3PIN, OUTPUT);
  digitalWrite(LED3PIN, LOW);
  //động cơ
 
  int Val_Analog = 0;
  int MotorSpeed = 0;
  pinMode(button, INPUT_PULLUP);
  pinMode(led4, OUTPUT);
  last = millis();
}

void loop() {
  
  read_UART();
  if (millis() - last >= 1000) {
    String strDHT = read_DHT();
    send2ESP(strDHT);
    last = millis();
  }
  //nút ấn đổi chiều
  int buttonStatus = digitalRead(button);
  Serial.println(buttonStatus);
  if (buttonStatus == LOW) {
    digitalWrite(led4, HIGH);
    myStepper.step(1024);  
  } else {
    digitalWrite(led4, LOW);
    myStepper.step(-1024);
    
  }
  //biến trở điều khiển tốc độ
  Val_Analog = analogRead(potentiometer);
  MotorSpeed = map(Val_Analog, 0, 1023, 0, 19);
  Serial.println(MotorSpeed);
  if (MotorSpeed > 0) 
  {
    myStepper.setSpeed(MotorSpeed);
    myStepper.step(1); 
  }
  //cảm biến ánh sáng
  int g = analogRead(gas);
  int s = digitalRead(as);
  if(g >=200){
    digitalWrite(led5,HIGH);
    digitalWrite(coi,HIGH);
  }
  else{
    digitalWrite(led5,LOW);
    digitalWrite(coi,LOW);
  }
  if(s == 1){
    digitalWrite(led6,HIGH);
  }
  else{
    digitalWrite(led6,LOW);
  }
  //cảm biến khí ga
  //đọc data
}

String read_DHT() {
  
  int g = analogRead(gas);
  int s = digitalRead(as);
  float h = dht.readHumidity();    //Đọc độ ẩm
  float t = dht.readTemperature(); //Đọc nhiệt độ
  String tmp = "";
  // Reading temperature or humidity takes about 250 milliseconds
  int a = analogRead(cambien);
  
  if (isnan(h) || isnan(t)|| isnan(a)|| isnan(s)||isnan(g)) {
    Serial.println("Failed to read from DHT sensor!");
    return tmp;
  }

  tmp = "h=" + String(h) + "&t=" + String(t) +"&a="+int(a) + "&s="+String(s)+ "&g="+String(g);
  Serial.print("H: ");
  Serial.print(h);
  lcd.setCursor(0, 0);
  lcd.print("h:");
  lcd.print(h);
  
  Serial.print("");
  Serial.print("T: ");
  Serial.print(t);
  lcd.setCursor(8, 0);
  lcd.print("T:");
  lcd.print(t);

  Serial.print("");
  Serial.print("A: ");
  Serial.print(a);
  lcd.setCursor(0, 1);
  lcd.print("A:");
  lcd.print(a);

  Serial.print("");
  Serial.print("As: ");
  Serial.print(s);
  lcd.setCursor(6, 1);
  lcd.print("a:");
   lcd.print(s);

  Serial.print("");
  Serial.print("G: ");
  Serial.print(g);
  lcd.setCursor(10, 1);
  lcd.print("G:");
  lcd.print(g);
  
  return tmp;
}

void read_UART() {
  while (mySerial.available()) {
    char inChar = (char)mySerial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }

    if (stringComplete) {
      Serial.print("Received data: ");
      Serial.print(inputString);

      if (inputString.indexOf("led1on") >= 0) {
        digitalWrite(LED1PIN, HIGH);
        Serial.println("LED1 ON");
      } else if (inputString.indexOf("led1off") >= 0) {
        digitalWrite(LED1PIN, LOW);
        Serial.println("LED1 OFF");
      }

      if (inputString.indexOf("led2on") >= 0) {
        digitalWrite(LED2PIN, HIGH);
        Serial.println("LED2 ON");
      } else if (inputString.indexOf("led2off") >= 0) {
        digitalWrite(LED2PIN, LOW);
        Serial.println("LED2 OFF");
      }

      if (inputString.indexOf("led3on") >= 0) {
        digitalWrite(LED3PIN, HIGH);
        Serial.println("LED3 ON");
      } else if (inputString.indexOf("led3off") >= 0) {
        digitalWrite(LED3PIN, LOW);
        Serial.println("LED3 OFF");
      }

      inputString = "";
      stringComplete = false;
    }
  }
}

void send2ESP(String str) {
  Serial.print("Send Data: ");
  Serial.println(str);
  mySerial.println(str);
  mySerial.flush();
}
