#include "PMTK_GPS.h"

PMTK_GPS::PMTK_GPS(Stream& stream): stream(stream), ackedCommand(*this,"PMTK001",1), ackResponse(*this,"PMTK001",2) {
}

bool PMTK_GPS::loop() {
	while(stream.available()>0) {
		if(encode(stream.read()))
			return(true);
	}
	return(false);
}

bool PMTK_GPS::readline(const time_t timeout) {
	time_t start=millis();
	while(millis()-start<timeout && stream.available()>0) {
		if(encode(stream.read())) {
			return(true);
		}
	}
	return(false);
}

unsigned int PMTK_GPS::checksum(const char* data, char parity) {
	for(size_t i(0); i<strlen(data); i++) {
		parity^=data[i];
	}
	return(parity);
}

bool PMTK_GPS::send(const unsigned int type) {
	char c[]={'\0'};
	return(send(type,c));
}

bool PMTK_GPS::send(const unsigned int type, const char* data) {
	stream.print("$");
	stream.print(PMTK_TALKERID);
	char parity=checksum(PMTK_TALKERID);
	char buf[6]; // Long enough for '*', checksum ("00") and linebreak ("\r\n"), plus null
	sprintf(buf,"%03d",type);
	stream.print(buf);
	parity^=checksum(buf);
	if(strlen(data)>0) {
		stream.print(',');
		parity^=',';
		stream.print(data);
		parity^=checksum(data);
	}
	sprintf(buf,"*%02X\r\n",parity);
	stream.print(buf);
	stream.flush();
	return(getAck(type));
}

bool PMTK_GPS::getAck(const unsigned int type) {
	// Clear
	ackedCommand.value(); ackResponse.value();
	int attempts=3;
	for(int i=attempts; i>0; i--) {
		readline(PMTK_READLINE_TIMEOUT);
		//if(ackedCommand.isUpdated() || ackResponse.isUpdated()) {
			//if((unsigned int)ackedCommand.value()==type) {
				stream.print(F("Acked command: "));
				stream.println(ackedCommand.value());
				stream.print(F("Ack response: "));
				stream.println(ackResponse.value());
				stream.print(F("Expected: "));
				stream.println(type);
				if(strcmp(ackResponse.value(),"3")==0)
					return(true);
			//}
		//}
	}
	return(false);
}

void PMTK_GPS::wakeup() {
	stream.print('\n');
}

bool PMTK_GPS::reset() {
	wakeup();
	send(104);
	// Wakeup command is not acked, so we accept any line as success
	return(readline());
}

bool PMTK_GPS::periodicMode(const unsigned int type, const uint32_t runTime, const uint32_t sleepTime, const uint32_t secondRunTime, const uint32_t secondSleepTime) {
	// Longest possible sentence
	// $PMTK225,0,518400000,518400000,51840000,518400000*1B
	// 1234567890123456789012345678901234567890123456789012
	//
	// But we only need to worry about the data fields:
	// 0,518400000,518400000,51840000,518400000
	// 1234567890123456789012345678901234567890
	char buf[41];
	sprintf(buf,"%d,%lu", type, runTime);
	if(sleepTime>0)
		sprintf(&buf[strlen(buf)],",%lu",sleepTime);
	if(secondRunTime>0)
		sprintf(&buf[strlen(buf)],",%lu",secondRunTime);
	if(secondSleepTime>0)
		sprintf(&buf[strlen(buf)],",%lu",secondSleepTime);
	return(send(225,buf));
}
