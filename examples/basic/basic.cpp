#include "PMTK_GPS.h"
#include <StreamDebugger.h>

// If you want to see all data to/from the GPS:
//StreamDebugger gpsSerial(Serial1,Serial);
//PMTK_GPS pmtk(gpsSerial);

// Normal operation
PMTK_GPS pmtk(Serial1);

void getResponse() {
	switch(pmtk.getResponse()) {
		case PMTK_RESPONSE_INVALID: Serial.println("Invalid"); break;
		case PMTK_RESPONSE_UNSUPPORTED: Serial.println("Unsupported"); break;
		case PMTK_RESPONSE_FAIL: Serial.println("Fail"); break;
		case PMTK_RESPONSE_SUCCESS: Serial.println("Success"); break;
		break;
	}
}

void setup() {
	while(!Serial);
	Serial.begin(115200);
	Serial1.begin(9600);
}

void loop() {
	Serial.println(F("Full cold start"));
	pmtk.fullColdStart();

	Serial.print(F("Normal periodic mode... "));
	pmtk.periodicMode(PMTK_PERIODIC_NORMAL);
	getResponse();

	Serial.print(F("Extend ephemeris time... "));
	pmtk.extendEphemerisTime(1,25,180000,60000);
	getResponse();

	Serial.print(F("Standby periodic mode... "));
	pmtk.periodicMode(PMTK_PERIODIC_STANDBY,3000,12000,18000,72000);
	getResponse();

	// Idle for a while
	uint32_t start=millis();
	while(millis()-start<15000) {
		pmtk.loop();
	}
}
