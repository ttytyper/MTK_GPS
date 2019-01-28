#include "PMTK_GPS.h"
#include <StreamDebugger.h>

// If you want to see all data to/from the GPS:
//StreamDebugger gpsSerial(Serial1,Serial);
//PMTK_GPS pmtk(gpsSerial);

// Normal operation
PMTK_GPS pmtk(Serial1);

void printAck(int ack) {
	switch(ack) {
		case PMTK_ACK_TIMEOUT: Serial.println("Timeout"); break;
		case PMTK_ACK_INVALID: Serial.println("Invalid"); break;
		case PMTK_ACK_UNSUPPORTED: Serial.println("Unsupported"); break;
		case PMTK_ACK_FAIL: Serial.println("Fail"); break;
		case PMTK_ACK_SUCCESS: Serial.println("Success"); break;
		break;
	}
}

void setup() {
	while(!Serial);
	Serial.begin(115200);
	Serial1.begin(9600);
}

void loop() {
	Serial.println(F("Hot start"));
	pmtk.hotStart();

	Serial.println(F("Set NMEA update rate"));
	pmtk.setNmeaUpdateRate(10000);

	Serial.print(F("Normal periodic mode... "));
	printAck(pmtk.periodicMode(PMTK_PERIODIC_NORMAL));

	Serial.print(F("Extend ephemeris time... "));
	printAck(pmtk.extendEphemerisTime(1,25,180000,60000));

	Serial.print(F("Single standby periodic mode... "));
	printAck(pmtk.periodicMode(PMTK_PERIODIC_STANDBY,3000,12000));

	Serial.print(F("Second standy periodic mode... "));
	printAck(pmtk.periodicMode(PMTK_PERIODIC_STANDBY,3000,12000,18000,72000));

	Serial.println(F("Idling for a while"));
	uint32_t start=millis();
	while(millis()-start<5000) {
		pmtk.loop();
	}
}
