//radio libraries
#include <string.h>
#include <util/crc16.h>
//sensor libraires
#include "Wire.h"
#include "I2Cdev.h"
#include "BMP085.h"
#include "MPU6050.h"
#include "HMC5883L.h"
//radio libraries
#include <SPI.h>
#include <SD.h>

#define RADIOPIN 9
#define GPSENABLE 3
#define INPUT_SIZE 11
#define LED_PIN 13

// class default I2C address is 0x77
// specific I2C addresses may be passed as a parameter here
// (though the BMP085 supports only one address)
BMP085 barometer;
MPU6050 accelgyro;
HMC5883L mag;
File scienceFile;
String tmp;
byte GPSBuffer[82];
byte GPSIndex=0;
String GPS_Time;
int inByte;
bool gps_updated;
bool blinkState = false;
String GPS_String;
String transmitString;
char datastring[80];
unsigned int CHECKSUM;
char checksum_str[6];
float temperature;
float pressure;
float altitude;
long timestamp;
int32_t lastMicros;
const float vcc = 5.0;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
float tmp36_temp;
float real_ax, real_ay, real_az;
float real_a;

float analog_averaging(int channel) {
  long sum = 0;
  for(int i = 0; i < 15; i++) {
    sum += analogRead(channel);
  }
  return (float)sum/15.0;
}
 
void setup() {
    pinMode(RADIOPIN,OUTPUT);
    setPwmFrequency(RADIOPIN, 1);
    pinMode(GPSENABLE, OUTPUT);
    digitalWrite(GPSENABLE, HIGH);
    Serial1.begin(9600);
    Serial.begin(9600);
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();

    // initialize device
    Serial.println("Initializing I2C devices...");
    barometer.initialize();
    accelgyro.initialize();
    mag.initialize();
    delay(100);

    // verify connection
    Serial.println("Testing device connections...");
    delay(100);
    Serial.println(barometer.testConnection() ? "BMP085 connection successful" : "BMP085 connection failed");
    delay(100);
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    delay(100);
    Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");
    delay(100);


    // configure LED pin for activity indication
    pinMode(LED_PIN, OUTPUT);

    // SD Card code
    //Serial.begin(9600);
    Serial.print("Initialising SD card...");

    //Remember to set the pin correctly. The shield we use is on pin 4! Don't listen to anyone that say otherwise!
    int pin = 4;
    pinMode(pin, OUTPUT);

    if (!SD.begin(pin)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");
}
 
void loop() {
    int tmp36_val = analog_averaging(A0);
    float tmp36_volt = ((float)tmp36_val / 1023.0) * vcc;
    tmp36_temp = ((tmp36_volt - 0.75) / 0.01) + 25.0;

    updateGPS();
    //if (gps_updated)
      //{
      // request temperature
      barometer.setControl(BMP085_MODE_TEMPERATURE);
      
      // wait appropriate time for conversion (4.5ms delay)
      lastMicros = micros();
      while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
  
      // read calibrated temperature value in degrees Celsius
      temperature = barometer.getTemperatureC();
  
      // request pressure (3x oversampling mode, high detail, 23.5ms delay)
      barometer.setControl(BMP085_MODE_PRESSURE_3);
      while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
  
      // read calibrated pressure value in Pascals (Pa)
      pressure = barometer.getPressure();
  
      // calculate absolute altitude in meters based on known pressure
      // (may pass a second "sea level pressure" parameter here,
      // otherwise uses the standard value of 101325 Pa)
      altitude = barometer.getAltitude(pressure);
  
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      mag.getHeading(&mx, &my, &mz);
  
  
      // display measured values if appropriate
      Serial.print("BMP085 temp: ");
      Serial.println(temperature); 
      Serial.print("BMP085 pres: ");
      Serial.println(pressure);
      Serial.print("BMP085 alt:  ");
      Serial.println(altitude);
      Serial.print("TMP36 temp:  ");
      Serial.println(tmp36_temp);
          
      real_ax = ax / (16384.0 / 9.81);
      real_ay = ay / (16384.0 / 9.81);
      real_az = az / (16384.0 / 9.81);
      Serial.print("Accel x,y,z: ");
      Serial.print(real_ax);
      Serial.print(", ");
      Serial.print(real_ay);
      Serial.print(", ");
      Serial.println(real_az);
      
      real_a = sqrt(real_ax*real_ax+real_ay*real_ay+real_az*real_az);
      Serial.print("Net accel:  ");
      Serial.println(real_a);
      Serial.print("Gyro x,y,z: ");
      Serial.print(gx);
      Serial.print(", ");
      Serial.print(gy);
      Serial.print(", ");
      Serial.println(gz);

      Serial.print("Magnetometer x,y,z: ");
      Serial.print(mx);
      Serial.print(", ");
      Serial.print(my);
      Serial.print(", ");
      Serial.println(mz);
      Serial.println("--------------------");

      /*
      snprintf(datastring, 80, "P=%ld, T=%ld", (long)pressure, (long)temperature);
      unsigned int CHECKSUM = gps_CRC16_checksum(datastring); // Calculates the checksum for this datastring
      char checksum_str[6];
      sprintf(checksum_str, "*%04X\n", CHECKSUM);
      strcat(datastring,checksum_str);
      rtty_txstring (datastring);
      updateScience();
      */

      transmitString = GPS_String + "," + String(mx) + "," + String(my) + "," + String(mz);
      
      transmitString.toCharArray(datastring,80); // Puts the text in the datastring
      Serial.print("GPS:");Serial.println(GPS_String);Serial.print("Timestamp:");Serial.println(millis());
      CHECKSUM = gps_CRC16_checksum(datastring); // Calculates the checksum for this datastring
      checksum_str[6];
      sprintf(checksum_str, "*%04X\n", CHECKSUM);
      strcat(datastring,checksum_str);
      rtty_txstring (datastring);
      gps_updated = false;
      //}
  }

  // Reads science data andwrites it to sd card.
void updateScience() {
  
  /*char science[INPUT_SIZE + 1];
  science = readScience();*/


  tmp = readScience();
  Serial.println(tmp);
  /*
  char science[1024];
  strncpy(science, tmp.c_str(), sizeof(science));
  science[sizeof(science) - 1] = 0;
  science[INPUT_SIZE] = 0;*/

  scienceFile = SD.open("HabSki.csv", FILE_WRITE);

  if (scienceFile){
    Serial.print("Writing Data...");

  writeScience (tmp);
  // This bit splits our string into separate numbers, and sends each to be written on the sd card.
  /*char* data =  strtok(science, ",");
  while (data != 0) {
    writeScience (data);
    data = strtok(0, ",");
  }*/
  

  scienceFile.close();
  
  Serial.println("\tData Updates Written");
  } else {
    Serial.println("Failed to open file!");
  }
}

// Somehow gets a string of data
// More specifically, takes all the data we have, and sticks it into a string in the same order
// as things are sent over the serial connection.
//
// This uses a lot of public variables... :)
String readScience() {
  //char* tempStr, pressureStr, altitudeStr;
  /*char* tempStr = floatToString(tempStr, temperature, 5);
  char* pressureStr = floatToString(pressureStr, pressure, 5);
  char* altitudeStr = floatToString(altitudeStr, altitude, 5);*/

  // Update timestamp to current time value.
  timestamp = millis();
  // First, all the atmospheric values from the BMP085 chip.
  return String(timestamp) + "," + String(temperature) + "," + String(pressure) + "," + String(altitude)
  // Now the temperature from TMP36 chip.
    + "," + String(tmp36_temp)
  // Now the accelearation along all 3 axes (BMP085).
    + "," + String(real_ax) + "," + String(real_ay) + "," + String(real_az)
  // The total acceleratiion (sum of the vectors) (BMP085).
    + "," + String(real_a)
  // Finally, the rotation along all 3 axes (BMP085).
    + "," + String(gx) + "," + String(gy) + "," + String(gz)
  // But there is more! also get the Magnetic field components (HMC5883L).
    + "," + String(mx) + "," + String(my) + "," + String(mz);
}



// Writes provided data into a file in the format we want.
void writeScience(String scienceData) {
  scienceFile.println (scienceData);
}

   
  void rtty_txstring (char * string) {
      /* Simple function to sent a char at a time to
      ** rtty_txbyte function.
      ** NB Each char is one byte (8 Bits)
    */
 
    char c;
    c = *string++;
 
    while ( c != '\0') {
        rtty_txbyte (c);
        c = *string++;
    }
}
void rtty_txbyte (char c) {
    /* Simple function to sent each bit of a char to
    ** rtty_txbit function.
    ** NB The bits are sent Least Significant Bit first
    **
    ** All chars should be preceded with a 0 and
    ** proceed with a 1. 0 = Start bit; 1 = Stop bit
    **
    */
 
    int i;
 
    rtty_txbit (0); // Start bit
 
    // Send bits for for char LSB first
 
    for (i=0;i<7;i++) { // Change this here 7 or 8 for ASCII-7 / ASCII-8
        if (c & 1) rtty_txbit(1);
        else rtty_txbit(0);
 
        c = c >> 1;
    }
    rtty_txbit (1); // Stop bit
    rtty_txbit (1); // Stop bit
}
 
void rtty_txbit (int bit) {
    if (bit) {
        // high
        analogWrite(RADIOPIN,110);
    }
    else {
        // low
        analogWrite(RADIOPIN,100);
    }
 
    // delayMicroseconds(3370); // 300 baud
    delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
    delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
    // largest value that will produce an accurate delay is 16383
    // See : http://arduino.cc/en/Reference/DelayMicroseconds
}
 
uint16_t gps_CRC16_checksum (char *string) {
    size_t i;
    uint16_t crc;
    uint8_t c;
 
    crc = 0xFFFF;
 
    // Calculate checksum ignoring the first two $s
    for (i = 2; i < strlen(string); i++) {
        c = string[i];
        crc = _crc_xmodem_update (crc, c);
    }
 
    return crc;
}
 
void setPwmFrequency(int pin, int divisor) {
    byte mode;
    if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
        switch(divisor) {
            case 1:
                mode = 0x01;
                break;
            case 8:
                mode = 0x02;
                break;
            case 64:
                mode = 0x03;
                break;
            case 256:
                mode = 0x04;
                break;
            case 1024:
                mode = 0x05;
                break;
            default:
                return;
        }
 
        if(pin == 5 || pin == 6) {
            TCCR0B = TCCR0B & 0b11111000 | mode;
        }
        else {
            TCCR1B = TCCR1B & 0b11111000 | mode;
        }
    }
    else if(pin == 3 || pin == 11) {
        switch(divisor) {
            case 1:
                mode = 0x01;
                break;
            case 8:
                mode = 0x02;
                break;
            case 32:
                mode = 0x03;
                break;
            case 64:
                mode = 0x04;
                break;
            case 128:
                mode = 0x05;
                break;
            case 256:
                mode = 0x06;
                break;
            case 1024:
                mode = 0x7;
                break;
            default:
                return;
        }
        TCCR2B = TCCR2B & 0b11111000 | mode;
    }
}

void updateGPS()
//Hour(Summer):Minutes:Seconds,Position,Altitude
{ 
  Serial.print ("Serial1: " + String(Serial1.available()));
  while (Serial1.available() > 0)
  {
    inByte = Serial1.read();
 
    Serial.write(inByte); // Output exactly what we read from the GPS to debug
 
    if ((inByte =='$') || (GPSIndex >= 80))
    {
      GPSIndex = 0;
    }
 
    if (inByte != '\r')
    {
      GPSBuffer[GPSIndex++] = inByte;
    }
 
    if (inByte == '\n' && (GPSBuffer[1] == 'G') && (GPSBuffer[2] == 'N') && (GPSBuffer[3] == 'G') && (GPSBuffer[4] == 'G') && (GPSBuffer[5] == 'A'))
    {
      int i,j,k,IntegerPart;
      long Longitude = 0;
      long Latitude = 0;
      unsigned int Altitude = 0;
      unsigned int GPS_Satellites = 0;
      String hemisphere;
      String Time;
      
      Time = String(GPSBuffer[7]-'0') + String(GPSBuffer[8]-'0'+1) + ':'
      + String(GPSBuffer[9]-'0') + String(GPSBuffer[10]-'0') + ':'
      + String(GPSBuffer[11]-'0') + String(GPSBuffer[12]-'0');

      for (i=7; i<GPSIndex; i++)
      {
        //negative longitude if west
        if (GPSBuffer[i] == 'W') hemisphere = "-";
        else if (GPSBuffer[i] == 'E') hemisphere = "";
      }
      //Serial.println(hemisphere);

      for (i=7, j=0, k=0; (i<GPSIndex) && (j<9); i++) // We start at 7 so we ignore the '$GNGGA,'
        {
          if (GPSBuffer[i] == ',')
          {
            j++;    // Segment index
            k=0;    // Index into target variable
            IntegerPart = 1;
          }
          else
          {
            if (j == 1)
            {
              // Latitude
              if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9') || (GPSBuffer[i] != '.'))
              {
                Latitude = Latitude * 10;
                Latitude += (int)(GPSBuffer[i] - '0');
              }
            }
            if (j == 3)
            {
              // Longitude
              if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9') || (GPSBuffer[i] != '.'))
              {
                Longitude = Longitude * 10;
                Longitude += (int)(GPSBuffer[i] - '0');
              }
            }
            else if (j == 8)
            {
              // Altitude
              if ((GPSBuffer[i] >= '0') && (GPSBuffer[i] <= '9') && IntegerPart)
              {
                Altitude = Altitude * 10;
                Altitude += (unsigned int)(GPSBuffer[i] - '0');
              }
              else
              {
                IntegerPart = 0;
              }
             }
            }
          }
    unsigned int LatitudeInt = Latitude/10000000;
    Latitude -= LatitudeInt*10000000;
    String GPS_Latitude = (String)LatitudeInt + '.' + (String)Latitude;
    unsigned int LongitudeInt = Longitude/10000000;
    Longitude -= LongitudeInt*10000000;
    String GPS_Longitude = hemisphere + (String)LongitudeInt + '.' + (String)Longitude;
    GPS_String = Time + "," + GPS_Latitude + "," + GPS_Longitude + "," + Altitude;
    gps_updated = true;
    //Serial.println(GPS_String);
    break;
    }
  }
}



