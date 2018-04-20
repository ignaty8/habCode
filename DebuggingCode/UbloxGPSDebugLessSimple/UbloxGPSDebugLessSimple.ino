#define GPSENABLE 2
#include <SoftwareSerial.h>

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
  if (Serial1.available()){
    Serial.write(Serial1.read());
  }
}
