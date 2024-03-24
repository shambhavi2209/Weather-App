//---------style guard --------
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

// ------header files----
#include <WiFi.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Servo.h>
#include "ThingSpeak.h"

//----network credentials

char* ssid = "wifiname";
char* pass = "wifipswd";
WiFiServer server(80);
WiFiClient client;

//------Thingspeak channel details
unsigned long myChannelNumber = 3;
const char* myWriteAPIKey = "----------------";

//------Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

#define DHTPIN 4 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

// Initializing the DHT11 sensor.
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
Servo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position
int LDR_VAL = 0;
int sensor = 34;
int ledPin = 19;

void setup() {
  Serial.begin(115200); //Initialize serial
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
   }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
//----nitialize dht11
  dht.begin();
  myservo.attach(13);  // attaches the servo on pin 13 to the servo object
  if(!bmp.begin()){
    Serial.println("BMP180 Not Found.");
    while(1){}
  }
  ThingSpeak.begin(client); // Initialize ThingSpeak
  digitalWrite(ledPin,LOW);
  myservo.write(pos);
}

void loop(){
  if((millis() - lastTime) > timerDelay) {
  delay(2500);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float p = bmp.readTemperature();
  float a = bmp.readAltitude();
  if(isnan(h) || isnan(t) || isnan(f)){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  LDR_VAL = analogRead(sensor);
  if(LDR_VAL == 0) {
    digitalWrite(ledPin,HIGH);
    Serial.print("LDR OUTPUT VALUE: ");
    Serial.println(LDR_VAL);
  }
  else {
    digitalWrite(ledPin,LOW);
    delay(1000);
    Serial.print("LDR OUTPUT VALUE: ");
    Serial.println(LDR_VAL);
  }
  Serial.print("Temperature (ºC): ");
  Serial.print(t);
  Serial.println("ºC");
  Serial.print("Humidity");
  Serial.println(h);
  Serial.print("Pressure = ");
  Serial.print(p);
  Serial.println("Pa");
  Serial.print("Altitude = ");
  Serial.print(a);
  Serial.println(" meters");
  if(t>20){
   for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);   
    delay(15);           // tell servo to go to position in variable 'pos'
    }
   }
  else {
   for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
     myservo.write(pos);              // tell servo to go to position in variable 'pos'                      
   }
   }
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, LDR_VAL);
  ThingSpeak.setField(4,p);
  ThingSpeak.setField(5,a);
  int x = ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
  if(x == 200){
      Serial.println("Channel update successful.");
   }
  else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
   }
  lastTime = millis();
   }
}
