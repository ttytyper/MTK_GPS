#include "PMTK_GPS.h"

PMTK_GPS::PMTK_GPS(Stream& stream, TinyGPSPlus gps): stream(stream), gps(gps) {
	TinyGPSCustom ackedCommand(gps,"PMTK001",1);
	TinyGPSCustom ackResponse(gps,"PMTK001",2);
}

bool PMTK_GPS::loop() {
	while(stream.available()>0) {
		if(gps.encode(stream.read()))
			return(true);
	}
	return(false);
}

void PMTK_GPS::readline() {
	while(stream.available()) {
		char c=stream.read();
		if(gps.encode(c)) {
			stream.write(c); // Echo
			return;
		}
	}
}

bool PMTK_GPS::readline(time_t timeout) {
	time_t start=millis();
	do {
		readline();
	} while(millis()-start < timeout);
	return(false);
}

unsigned int PMTK_GPS::checksum(const char* data, char parity) {
	for(size_t i(0); i<strlen(data); i++) {
		parity^=data[i];
	}
	return(parity);
}

bool PMTK_GPS::send(unsigned int type) {
	char c[]={'\0'};
	return(send(type,c));
}

bool PMTK_GPS::send(unsigned int type, char* data) {
	stream.print("$PMTK");
	char buf[8];
	sprintf(buf,"%03d",type);
	stream.print(buf);
	sprintf(buf,"*%02X\r\n",checksum(buf,2));
	stream.print(buf);
	stream.flush();
	return(getAck(type));
}

bool PMTK_GPS::getAck(unsigned int type) {
	// Clear
	ackedCommand.value(); ackResponse.value();
	time_t start=millis();
	while(millis()-start<PMTK_ACK_TIMEOUT) {
		// BUG: We could potentially run for twice the timeout
		readline(PMTK_ACK_TIMEOUT);
		//if(ackedCommand.isUpdated() || ackResponse.isUpdated()) {
			if((unsigned int)ackedCommand.value()==type) {
				return(((unsigned int)ackResponse.value()==0));
			}
		//}
	}
	return(false);
}

void PMTK_GPS::wakeup() {
	stream.print('\n');
}

bool PMTK_GPS::reset() {
	wakeup();
	return(send(104));
}

bool PMTK_GPS::periodicMode(unsigned int type,uint32_t runTime, uint32_t sleepTime, uint32_t secondRunTime, uint32_t secondSleepTime) {
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
	return(send(223,buf));
}
