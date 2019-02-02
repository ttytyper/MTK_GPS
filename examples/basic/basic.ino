/**
 * Basic example showing some of the functions available in MTK_GPS
 */
#include "MTK_GPS.h"

// If you want to see all data to/from the GPS:
//#include <StreamDebugger.h>
//StreamDebugger gpsSerial(Serial1,Serial);
//MTK_GPS gps(gpsSerial);

// Normal operation
MTK_GPS gps(Serial1);

void printAck(int ack) {
	switch(ack) {
		case MTK_ACK_TIMEOUT: Serial.println("Timeout"); break;
		case MTK_ACK_INVALID: Serial.println("Invalid"); break;
		case MTK_ACK_UNSUPPORTED: Serial.println("Unsupported"); break;
		case MTK_ACK_FAIL: Serial.println("Fail"); break;
		case MTK_ACK_SUCCESS: Serial.println("Success"); break;
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
	gps.hotStart();

	Serial.println(F("Set NMEA update rate"));
	gps.setNmeaUpdateRate(10000);

	Serial.print(F("Normal periodic mode... "));
	printAck(gps.periodicMode(MTK_PERIODIC_NORMAL));

	Serial.print(F("Extend ephemeris time... "));
	printAck(gps.extendEphemerisTime(1,25,180000,60000));

	Serial.print(F("Single standby periodic mode... "));
	printAck(gps.periodicMode(MTK_PERIODIC_STANDBY,3000,12000));

	Serial.print(F("Second standy periodic mode... "));
	printAck(gps.periodicMode(MTK_PERIODIC_STANDBY,3000,12000,18000,72000));

	Serial.println(F("Idling for a while"));
	uint32_t start=millis();
	while(millis()-start<5000) {
		gps.loop();
	}
}
