#include <SPI.h>
#include <SD.h>
 
File scienceFile;

#define INPUT_SIZE 11

void setup() {
  Serial.begin(9600);
  Serial.print("Initialising SD card...");
  delay(100);
  //Remember to set the pin correctly. The shield we use is on pin 4! Don't listen to anyone that say otherwise!
  int pin = 4;
  pinMode(pin, OUTPUT);
  delay(100);
  if (!SD.begin(pin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  delay(100);
}

void loop() {
  updateScience();

  delay(10000);

}

// Reads science data andwrites it to sd card.
void updateScience() {

  /*char science[INPUT_SIZE + 1];
  science = readScience();*/

  String tmp;
  tmp = readScience();
  char science[1024];
  strncpy(science, tmp.c_str(), sizeof(science));
  science[sizeof(science) - 1] = 0;

  science[INPUT_SIZE] = 0;

  scienceFile = SD.open("HabSci.csv", FILE_WRITE);

  if (scienceFile){
    Serial.print("Writing Data...");

  // This bit splits our string into separate numbers, and sends each to be written on the sd card.
  char* data =  strtok(science, ",");
  while (data != 0) {
    writeScience (data);

    data = strtok(0, ",");
  }

  scienceFile.close();
  
  Serial.println("\tData Updates Written");
  } else {
    Serial.println("Failed to open file!");
  }
}

// Somehow gets a string of data
String readScience() {
  return "Test of DHOOM";
}

// Writes provided data into a file in the format we want.
void writeScience(char* scienceData) {
  scienceFile.println (scienceData);
}


