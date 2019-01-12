#include "PMTK_GPS.h"

PMTK_GPS::PMTK_GPS(Stream& stream) : stream(stream) {
}

bool PMTK_GPS::loop() {
	while(stream.available()>0) {
		if(encode(stream.read()))
			return(true);
	}
	return(false);
}

void PMTK_GPS::readline() {
	while(stream.available()) {
		if(encode(stream.read()))
			return;
	}
}

bool PMTK_GPS::readline(time_t timeout) {
	time_t start=millis();
	do {
		readline();
	} while(millis()-start < timeout);
	return(false);
}

unsigned int PMTK_GPS::checksum(char* data) {
	char parity(0);
	for(size_t i(0); i<strlen(data); i++) {
		parity^=data[i];
	}
	return(parity);
}

void PMTK_GPS::send(unsigned int type, char* data) {
	stream.print("$PMTK");
	stream.print(type);
	stream.print(",");
	stream.print(data);
	stream.print("*");
	char buf[3];
	sprintf(buf,"%02x",checksum(data));
	stream.println(buf);
}

void PMTK_GPS::periodicMode(unsigned int type,uint32_t runTime, uint32_t sleepTime, uint32_t secondRunTime, uint32_t secondSleepTime) {
	// Longest possible sentence
	// $PMTK225,0,518400000,518400000,51840000,518400000*1B
	// 1234567890123456789012345678901234567890123456789012
	//
	// But we only need to worry about the data fields:
	// 0,518400000,518400000,51840000,518400000
	// 1234567890123456789012345678901234567890
	char buf[41];
	sprintf(buf,"%d,%lu,%lu,%lu,%lu",
			type,
			runTime,
			sleepTime,
			secondRunTime,
			secondSleepTime
	);
	send(225,buf);
}
