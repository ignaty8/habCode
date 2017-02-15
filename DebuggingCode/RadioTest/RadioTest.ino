/*  NTX2 Radio Test Part 1
 
    Toggles the NTX2 high and low to generate a tone pattern to ascertain 
    if the radio and associated circuitry is working. 
 
    Created 2012 by M0UPU as part of a UKHAS Guide on linking NTX2 Modules to Arduino.
 
    http://ukhas.org.uk
*/ 
 
#define RADIOPIN 9
 
void setup()
{
pinMode(RADIOPIN,OUTPUT);
}
 
void loop()  
{                    
digitalWrite(RADIOPIN, HIGH);                    
delay(100);                    
digitalWrite(RADIOPIN, LOW);                    
delay(100);          
}
