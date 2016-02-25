#include <SPI.h>

#include <RFM98W_library.h>
RFMLib radio =RFMLib(PA4,PC15,255,255);
#define nss PA4
#define LORA_CTRL1 PA0
#define LORA_CTRL2 PA1 

void setup(){
  pinMode(PA4, OUTPUT);
  digitalWrite(PA4, HIGH);
  pinMode(LORA_CTRL1, OUTPUT);
  pinMode(LORA_CTRL2, OUTPUT);
  digitalWrite(LORA_CTRL1, LOW);
  digitalWrite(LORA_CTRL2, LOW);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  Serial.begin(38400);
  byte my_config[6] = {0x44,0x84,0x88,0xAC,0xCD, 0x08};
  radio.configure(my_config);
}

void loop(){
  if(radio.rfm_status == 0){
    radio.beginRX(); 
    attachInterrupt(7,RFMISR,RISING);
  }

  if(radio.rfm_done){
        Serial.println("Ending");   
    RFMLib::Packet rx;
    radio.endRX(rx);
   for(byte i = 0;i<rx.len;i++){
     Serial.print(rx.data[i]);
     Serial.print("  ");
   }
   Serial.println();
  }
  
}

void RFMISR(){
  Serial.println("interrupt");
 radio.rfm_done = true; 
}


