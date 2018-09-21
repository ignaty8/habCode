/*
 Name:		ICSEDSTest.ino
 Created:	2/26/2017 1:35:24 PM
 Author:	robert
*/
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <util/crc16.h>

#include <SD.h>
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

#define LEDPIN 13
#define RADIOPIN 9
//#define GPSTXPIN 4
//#define GPSRXPIN 3
#define GPSENABLE 2
char datastring[140];
char floatBuffer[10];
String csvString;
String timeActual;

int CS_PIN = 10;

String callsign = "NEMO";

File file;

/* Assign a unique ID to the sensors */
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);


/* Update this with the correct SLP for accurate altitude measurements */
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;

//SoftwareSerial GPSSERIAL(GPSTXPIN, GPSRXPIN);
#define GPSSERIAL Serial1
TinyGPSPlus gps;
// the setup function runs once when you press reset or power the board
/*struct PosData {
	float lat, longd, alt;
};
struct PosData telemetry;
*/
double posLat, posLongd, posAlt;
void setup() {
  pinMode(RADIOPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(GPSENABLE, OUTPUT);
  digitalWrite(GPSENABLE, HIGH);

  Serial.begin(38400);
  while (!Serial) {

  }
  //Serial1.begin(9600);
  //GPS Initalization Initalize software serial and tie to serial
  //telemetry = { 0f,0f,0f };

  initializeSD();
  createFile("telem.txt");
  closeFile();

  //initSensors();
}
// the loop function runs over and over again until power down or reset
void loop() {
  //delay(5000);
  
  Serial1.begin(9600);
  for(int k = 0; k < 200; k++){
    delay(10);
    while (Serial1.available())
    {
      //Serial.write(Serial1.read());
      //delay(100);
      //send the serial data to buffer
      //clean up data (?)
      //serial end?
      //gps.encode(buffer)
      gps.encode(Serial1.read());
    }
  }
  Serial1.end();
  
  //Serial.println(gps.location.lat(), 6);
  //int data = Serial1.read();
  //Serial.println("data: " + data);
  //if () {
    posAlt = gps.altitude.meters();
    //Serial.println(gps.location.lat(), 6);
    posLat = gps.location.lat();
    posLongd = gps.location.lng();

    timeActual = String(gps.time.hour()) + ':' + String(gps.time.minute()) + ':' + String(gps.time.second());
  //}
  
  //sprintf(datastring, "Alt=%f ,Lat=%f , Long = %f", telemetry.alt,telemetry.lat,telemetry.longd);
  //snprintf(datastring, 80, "%.6f,%.6f,%.6f", posLat,posLongd,posAlt);
  //Serial.println(datastring); //return;
  //sprintf(datastring, 80, "P=%ld, T=%ld");//, (long)pressure, (long)temperature);

  csvString = "$$" + callsign + ',' + String(millis());
  csvString = csvString + ',' + timeActual;
  csvString += ',' + String(posLat, 6);
  csvString += ',' + String(posLongd,6);
  csvString += ',' + String(posAlt,6);
  csvString.toCharArray(datastring,140);
  unsigned int CHECKSUM = gps_CRC16_checksum(datastring);  // Calculates the checksum for this datastring
  char checksum_str[6];
  sprintf(checksum_str, "*%04X\n", CHECKSUM);
  strcat(datastring, checksum_str);
  //Serial.println(gps.altitude.meters());
  //Serial.println(gps.location.lat());
  //Serial.println(gps.location.lng());
  Serial.println("Transmitting Data:");
  Serial.println(datastring);
  rtty_txstring(datastring);

  //Serial.println(csvString);
  
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t   orientation;
  
  //TODO: add timer so this stupid library doesn't interrupt execution if the sensorgets disconnected
  //Srsly who the HELL CODED THIS LIBRARY?!
  /* Calculate pitch and roll from the raw accelerometer data */
  if(accel.begin()){
  accel.getEvent(&accel_event);
  if (dof.accelGetOrientation(&accel_event, &orientation))
  {
    /* 'orientation' should have valid .roll and .pitch fields */
    csvString = csvString + ',' + orientation.roll;
    csvString = csvString + ',' + orientation.pitch;
  }
  }

  /* Calculate the heading using the magnetometer */
  if(mag.begin()){
  mag.getEvent(&mag_event);
  if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation))
  {
  }

  if (dof.magTiltCompensation(SENSOR_AXIS_Z, &mag_event, &accel_event))
  {
    csvString = csvString + ',' + String(orientation.heading);
  }
  else
  {
    // Oops ... something went wrong (probably bad data)
  }
  }
  openFile("telem.txt");
  char* fileString;
  //csvString.toCharArray(fileString,);
  writeToFile(csvString);
  closeFile();
  //Serial.println(fileString);
  //Serial.println(csvString);
  Serial.println();

  if(posAlt < 500 && gps.time.hour() >= 19){
    digitalWrite(LEDPIN, HIGH);
  } else {
    digitalWrite(LEDPIN, LOW);
  }
  delay(2000);
}

void storeTelemetry() {

}

void initSensors()
{
  if (!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    //while(1);
  }
  if (!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    //while(1);
  }
}

void initializeSD()
{
  Serial.println("Initializing SD card...");
  pinMode(CS_PIN, OUTPUT);

  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
}

int createFile(char filename[])
{
  file = SD.open(filename, FILE_WRITE);

  if (file)
  {
    Serial.println("File created successfully.");
    return 1;
  } else
  {
    Serial.println("Error while creating file.");
    return 0;
  }
}

int writeToFile(String text)
{
  if (file)
  {
    file.println(text);
    Serial.println("Writing to file: ");
    Serial.println(text);
    return 1;
  } else
  {
    Serial.println("Couldn't write to file");
    return 0;
  }
}

void closeFile()
{
  if (file)
  {
    file.close();
    Serial.println("File closed");
  }
}

int openFile(char filename[])
{
  file = SD.open(filename);
  if (file)
  {
    Serial.println("File opened with success!");
    return 1;
  } else
  {
    Serial.println("Error opening file...");
    return 0;
  }
}

void rtty_txstring(char * string)
{

  /* Simple function to sent a char at a time to
  ** rtty_txbyte function.
  ** NB Each char is one byte (8 Bits)
  */

  char c;

  c = *string++;

  while (c != '\0')
  {
    rtty_txbyte(c);
    c = *string++;
  }
}

void rtty_txbyte(char c)
{
  /* Simple function to sent each bit of a char to
  ** rtty_txbit function.
  ** NB The bits are sent Least Significant Bit first
  **
  ** All chars should be preceded with a 0 and
  ** proceded with a 1. 0 = Start bit; 1 = Stop bit
  **
  */

  int i;

  rtty_txbit(0); // Start bit

  // Send bits for for char LSB first

  for (i = 0; i < 7; i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
  {
    if (c & 1) rtty_txbit(1);

    else rtty_txbit(0);

    c = c >> 1;

  }
  rtty_txbit(1); // Stop bit
  rtty_txbit(1); // Stop bit
}

void rtty_txbit(int bit)
{
  if (bit)
  {
    // high
    digitalWrite(RADIOPIN, HIGH);
  }
  else
  {
    // low
    digitalWrite(RADIOPIN, LOW);

  }

  //                  delayMicroseconds(3370); // 300 baud
  delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
  delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
  // largest value that will produce an accurate delay is 16383
  // See : http://arduino.cc/en/Reference/DelayMicroseconds

}

uint16_t gps_CRC16_checksum(char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;

  crc = 0xFFFF;

  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update(crc, c);
  }

  return crc;
}

