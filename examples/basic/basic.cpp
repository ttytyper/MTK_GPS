#define PMTK_DEBUG
#include "PMTK_GPS.h"
#include <StreamDebugger.h>
StreamDebugger gpsSerial(Serial1,Serial);

TinyGPSPlus gps;
TinyGPSCustom ackedCommand(gps,"PMTK001",1);
TinyGPSCustom ackResponse(gps,"PMTK001",2);
PMTK_GPS pmtk(gpsSerial, gps);
void setup() {
	while(!Serial);
	Serial.begin(115200);
	Serial1.begin(9600);
}

void loop() {
	pmtk.reset();
	Serial.print(F("Acked command: "));
	Serial.println(ackedCommand.value());
	Serial.print(F("Ack response: "));
	Serial.println(ackResponse.value());
	if(pmtk.periodicMode(PMTK_PERIODIC_STANDBY,3000,12000,18000,72000))
		Serial.println(F("OK"));
	else
		Serial.println(F("NAK"));
	Serial.print(F("Acked command: "));
	Serial.println(ackedCommand.value());
	Serial.print(F("Ack response: "));
	Serial.println(ackResponse.value());
	uint32_t start=millis();
	while(millis()-start<5000) {
		pmtk.readline();
	}
}
