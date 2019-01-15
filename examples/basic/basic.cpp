#define PMTK_DEBUG
#include "PMTK_GPS.h"
#include <StreamDebugger.h>
StreamDebugger gpsSerial(Serial1,Serial);

PMTK_GPS pmtk(gpsSerial);
void setup() {
	while(!Serial);
	Serial.begin(115200);
	Serial1.begin(9600);
}

void loop() {
	// Reset, read a few lines, then throw out some commands and see if they stick
	//pmtk.reset();
	uint32_t start=millis();
	while(millis()-start<5000) {
		pmtk.readline();
	}
	/*if(pmtk.periodicMode(PMTK_PERIODIC_NORMAL))
		Serial.println("OK");
	else
		Serial.println("NAK");*/

	/*if(pmtk.extendEphemerisTime(1,25,180000,60000))
		Serial.println("OK");
	else
		Serial.println("NAK");*/

	if(pmtk.periodicMode(PMTK_PERIODIC_STANDBY,3000,12000,18000,72000))
		Serial.println("OK");
	else
		Serial.println("NAK");
}
