#include "PMTK_GPS.h"

PMTK_GPS gps(Serial);
void setup() {
	gps.periodicMode(PMTK_PERIODIC_NORMAL,500,1000);
}

void loop() {
	gps.loop();
}
