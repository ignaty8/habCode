

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#include "Wire.h"

// I2Cdev and BMP085 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "BMP085.h"
#include "MPU6050.h"

#define RADIOPIN 9
 
#include <string.h>
#include <util/crc16.h>
 
char datastring[80];

// class default I2C address is 0x77
// specific I2C addresses may be passed as a parameter here
// (though the BMP085 supports only one address)
BMP085 barometer;
MPU6050 accelgyro;

float temperature;
float pressure;
float altitude;
int32_t lastMicros;

#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

const float vcc = 5.0;

float analog_averaging(int channel) {
  long sum = 0;
  for(int i = 0; i < 15; i++) {
    sum += analogRead(channel);
  }
  return (float)sum/15.0;
}

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();
    pinMode(RADIOPIN,OUTPUT);
    setPwmFrequency(RADIOPIN, 1);
    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(38400);

    // initialize device
    Serial.println("Initializing I2C devices...");
    barometer.initialize();
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(barometer.testConnection() ? "BMP085 connection successful" : "BMP085 connection failed");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // configure LED pin for activity indication
    pinMode(LED_PIN, OUTPUT);
}

int16_t ax, ay, az;
int16_t gx, gy, gz;

void loop() {
      int tmp36_val = analog_averaging(A0);
    float tmp36_volt = ((float)tmp36_val / 1023.0) * vcc;
    float tmp36_temp = ((tmp36_volt - 0.75) / 0.01) + 25.0;
  
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


    // display measured values if appropriate
    Serial.print("BMP085 temp: ");
    Serial.println(temperature); 
    Serial.print("BMP085 pres: ");
    Serial.println(pressure);
    Serial.print("BMP085 alt:  ");
    Serial.println(altitude);
    Serial.print("TMP36 temp:  ");
    Serial.println(tmp36_temp);
        
    float real_ax = ax / (16384.0 / 9.81);
    float real_ay = ay / (16384.0 / 9.81);
    float real_az = az / (16384.0 / 9.81);
    Serial.print("Accel x,y,z: ");
    Serial.print(real_ax);
    Serial.print(", ");
    Serial.print(real_ay);
    Serial.print(", ");
    Serial.println(real_az);
    
    float real_a = sqrt(real_ax*real_ax+real_ay*real_ay+real_az*real_az);
    Serial.print("Net accel:  ");
    Serial.println(real_a);
    Serial.print("Gyro x,y,z: ");
    Serial.print(gx);
    Serial.print(", ");
    Serial.print(gy);
    Serial.print(", ");
    Serial.println(gz);
        Serial.println("--------------------");
    snprintf(datastring, 80, "P=%ld, T=%d", (long)pressure, (long)temperature);
    unsigned int CHECKSUM = gps_CRC16_checksum(datastring); // Calculates the checksum for this datastring
    char checksum_str[6];
    sprintf(checksum_str, "*%04X\n", CHECKSUM);
    strcat(datastring,checksum_str);
    rtty_txstring (datastring);
  //  delay(1000);
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
