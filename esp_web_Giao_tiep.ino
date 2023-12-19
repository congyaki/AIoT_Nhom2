#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
String webPage =
{
"<!DOCTYPE html>"
"<html>"
"<head>"
" <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
" <title>Nhóm 3 IOT</title>"
" <meta name='viewport' content='width=device-width, initial-scale=1'>"
" <style>"
" .b{width: 100px;height: 40px;font-size: 21px;color:#FFF;background-color:#4caf50;border-radius: 10px;}"
" .t{width: 100px;height: 40px;font-size: 21px;color:#FFF;background-color:#f44336;border-radius: 10px;}"
" </style>"
"</head>"
"<body>"
"<div style='width: 330px;height: auto;margin: 0 auto;margin-top: 70px'>"
"<h1 align='center'>Điều khiển thiết bị IoT</h1>"
" <table align='center'>"
" <tr>"
" <td><a href='/led1on'><button class='b'>Bật 1</button></a><td>"
" <td><a href='/led1off'><button class='t'>Tắt 1</button></a><td>"
" <tr>"
" <tr>"
" <td><a href='/led2on'><button class='b'>Bật 2</button></a><td>"
" <td><a href='/led2off'><button class='t'>Tắt 2</button></a><td>"
" <tr>"
" <tr>"
" <td><a href='/led3on'><button class='b'>Bật 3</button></a><td>"
" <td><a href='/led3off'><button class='t'>Tắt 3</button></a><td>"
" <tr>"
" </table>"
"</div>"
"</body>"
"</html>"
};
const byte RX = 12;
const byte TX = 14;
SoftwareSerial mySerial(RX, TX);

// http
HTTPClient http;  //Declare an object of class HTTPClient
//WiFiClient client;

// Cloud dweet.io
char server[] = "www.dweet.io";
WiFiClient client;

// Replace with your network credentials
const char* ssid = "thang";
const char* password = "12345678";

bool stringComplete = false; // whether the string is complete
String outputStringDHT="";
// Set Local Host

String outputString="", inputString = ""; // a String to hold incoming data

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress staticIP(192,168,5,100); // Local IP
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer webServer(80);

long last = 0;
void setup() {
  Serial.begin(9600);
  Serial.println("");
  Serial.print("Setting Wi-Fi Mode:");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print(ssid); 
  Serial.println("");
  Serial.println("Connecting");
  Serial.println(WiFi.localIP());
  WiFi.config(staticIP, gateway, subnet);
  
  mySerial.begin(9600); 
  while(!mySerial){
  ;}
  pinMode(4,OUTPUT);
  digitalWrite(4, HIGH);
  last = millis();
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  } 
  
  outputString="";
  outputStringDHT="";
  webServer.on("/", TrangChu);
  webServer.on("/led1on", led1on);
  webServer.on("/led1off", led1off);
  webServer.on("/led2on", led2on);
  webServer.on("/led2off", led2off);
  webServer.on("/led3on", led3on);
  webServer.on("/led3off", led3off);
  webServer.begin();
  
}
void loop() {
  read_UART();
  webServer.handleClient(); // Điều khiển thiết bị
  if(millis()-last>=1000){
    // Send to Arduino UNO
    send2UNO(outputString);
    // send DHT data to cloud: dweet.io
    httpRequest(outputStringDHT);
    last = millis();
  }
}
void httpRequest(String thang) {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    if (client.connect(server, 80)) {
      String s = "GET /dweet/for/luuquyetthang?";
      s.concat(thang);
      Serial.println(s);
      client.println(s);
      client.println("Host: www.dweet.io");
      client.println("Connection: close");
      client.println();
    }
  }
}
void read_UART(){
  while (mySerial.available()) {
  // get the new byte:
  char inChar = (char)mySerial.read();
  // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
    stringComplete = true;
  }
  if(stringComplete==true){
    Serial.print("Da nhan UNO: ");
    Serial.println(inputString);
    Serial.print("Received: ");
    Serial.print(inputString);
    outputStringDHT= inputString;
    inputString="";
    stringComplete=false;
    }
  }
 }
void send2UNO(String str){
  Serial.print("Send Data: ");
  Serial.println(str);
  delay(200);
  mySerial.println(str);
  mySerial.flush();
}
void TrangChu()
{
  webServer.send(200, "text/html", webPage);
}
void led1on()
{
  Serial.println("bat LED 1");
  send2UNO("led1on");
  webServer.send(200, "text/html", webPage);
}
void led1off()
{
  Serial.println("Tắt LED 1");
  send2UNO("led1off");
  webServer.send(200, "text/html", webPage);
}
void led2on()
{Serial.println("bat LED 2");
  send2UNO("led2on");
  webServer.send(200, "text/html", webPage);
}
void led2off()
{
  send2UNO("led2off");
  Serial.println("Tắt LED 2");
  webServer.send(200, "text/html", webPage);
}
void led3on()
{Serial.println("bat LED 3");
  send2UNO("led3on");
  webServer.send(200, "text/html", webPage);
}
void led3off()
{Serial.println("Tắt LED 3");
  send2UNO("led3off");
  webServer.send(200, "text/html", webPage);
}
