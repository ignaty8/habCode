/*
 Name:		ICSEDSTest.ino
 Created:	2/26/2017 1:35:24 PM
 Author:	robert
*/
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <string>
#define LEDPIN 13
#define RADIOPIN 9
#define GPSTXPIN 4
#define GPSRXPIN 3
#define GPSENABLE 2
char outputDate[80];

SoftwareSerial GPSSerial(GPSTXPIN, GPSRXPIN);
TinyGPSPlus gps;
// the setup function runs once when you press reset or power the board
struct PosData {
	float lat, longd, alt;
};
PosData telemetry;
void setup() {
	pinMode(OUTPUT, RADIOPIN);
	pinMode(OUTPUT, LEDPIN);
	pinMode(OUTPUT, GPSENABLE);
	digitalWrite(GPSENABLE, HIGH);
	GPSSerial.begin(9600);
	//GPS Initalization Initalize software serial and tie to serial
	data = { 0,0,0 };
}
// the loop function runs over and over again until power down or reset
void loop() {
	while (GPSSerial.available()) {
		int data = GPSSerial.read();
		if (gps.encode(data)) {
			telemetry.alt = gps.altitude.meters;
			telemetry.lat = gps.location.rawLat;
			telemetry.longd = gps.location.rawLng;
		}
	}
	sprintf(datastring, gpsDataToString(telemetry));
	unsigned int CHECKSUM = gps_CRC16_checksum(datastring);  // Calculates the checksum for this datastring
	char checksum_str[6];
	sprintf(checksum_str, "*%04X\n", CHECKSUM);
	strcat(datastring, checksum_str);
	rtty_txstring(datastring);
	delay(2000);
}
std::string gpsDataToString(PosData data) {
	return "ALT=" + data.alt + ",LAT=" + data.lat + ",LONG=" + data.longd;
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

	for (i = 0; i<7; i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
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

