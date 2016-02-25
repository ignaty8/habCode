#include <SPI.h>

#include <RFM98W_library.h>
RFMLib radio =RFMLib(10,2,255,255);
#define nss 10
#define nss2 4

const char * chars = "?0123456789,.-+ !ABCDEHIMNOPRSTW";
const int n_chars = 32;
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
char charBuf[256];


int posInCharBuf = 0;

//Puts the encoded 5-bit string into buf. Returns the number of bytes
int encode_5bit(const char *str, uint8_t buf[]) {
  int bitPosInBuf = 0;
  
  while(*str != 0) {
    int charVal = 0;
    for(int i = 0; i < n_chars; i++) {
      if(chars[i] == *str) {
        charVal = i;
        break;
      }
    }
    
    for(int bit = 4; bit >= 0; bit--) {
     if((bitPosInBuf % 8) == 0) buf[bitPosInBuf / 8] = 0;
      if((charVal & (1 << bit)) != 0) {
        //set bit in packed stream
        buf[bitPosInBuf / 8] |= (1 << (7 - (bitPosInBuf % 8)));
      } 
      
      bitPosInBuf++;
    }
    
    str++;
  }
  
  return (bitPosInBuf + 7) / 8;
}


void loop(){
    if(Serial.available()) {
      char c = Serial.read();
      if(c=='\n') {
            charBuf[posInCharBuf] = '\0';
            Serial.println("BEGIN");
            RFMLib::Packet p;
            p.len = encode_5bit(charBuf, p.data);
            radio.beginTX(p); 
            attachInterrupt(0,RFMISR,RISING);
            posInCharBuf = 0;
      } else {
        charBuf[posInCharBuf++] = c;
      }
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


