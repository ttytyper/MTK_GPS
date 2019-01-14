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
	//pmtk.reset();
	if(pmtk.periodicMode(PMTK_PERIODIC_STANDBY,3000,12000,18000,72000))
		Serial.println(F("OK"));
	else
		Serial.println(F("NAK"));
	uint32_t start=millis();
	while(millis()-start<5000) {
		pmtk.readline();
	}
}
