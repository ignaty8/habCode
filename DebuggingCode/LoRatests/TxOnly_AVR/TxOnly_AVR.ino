#include <SPI.h>

#include <RFM98W_library.h>
RFMLib radio =RFMLib(10,2,255,255);
#define nss 10
#define nss2 4

void setup(){
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(nss2, OUTPUT);
  digitalWrite(nss2, HIGH);
  SPI.begin();
  
  Serial.begin(38400);
  byte my_config[6] = {0x44,0x84,0x88,0xc0,0xfc, 0x08};
  radio.configure(my_config);
    radio.wRFM(0x06, 0x85); 
  delay(3000);
}
int i = 0;
void loop(){
 // Serial.println(radio.rRFM(0x42),HEX);
  //delay(50);
  if(radio.rfm_status ==0){
    Serial.println("BEGIN");
    RFMLib::Packet p;
    p.data[0]=i >> 8;
    p.data[1]=i & 0xFF;
    i++;
    p.len = 2;
    radio.beginTX(p); 
    
    //while((digitalRead(PC15)!=1) && ((millis() - startTime) < 2000));
  //  radio.rfm_done = true;  

  
    attachInterrupt(0,RFMISR,RISING);

  }

  if(radio.rfm_done){
        Serial.println("Ending");   
    radio.endTX();
  }
  
}

void RFMISR(){
  Serial.println("interrupt");
 radio.rfm_done = true; 
}


