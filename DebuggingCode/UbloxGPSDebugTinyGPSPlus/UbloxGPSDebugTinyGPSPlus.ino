#define GPSENABLE 2
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

SoftwareSerial gpsSerial(50,51);
 
void setup() {
 pinMode(GPSENABLE, OUTPUT);
 digitalWrite(GPSENABLE, HIGH);

 Serial.begin(38400);
 while(!Serial){
  
 }
 Serial1.begin(9600);
 Serial.print("!");
}
 
void loop() {
  /*if (Serial1.available()){
    Serial.write(Serial1.read());
  }*/
  while(Serial1.available()){
    gps.encode(Serial1.read());
  }
  Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
  Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
  Serial.print("ALT=");  Serial.println(gps.altitude.meters());
  delay(500);
}
