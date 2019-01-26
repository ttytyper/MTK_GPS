#include "PMTK_GPS.h"

PMTK_GPS::PMTK_GPS(Stream& stream): stream(stream), ackType(*this,"PMTK001",1), ackResponse(*this,"PMTK001",2) {
}

int PMTK_GPS::getResponse() {
	return(atoi(ackResponse.value()));
}

bool PMTK_GPS::loop() {
	while(stream.available()>0) {
		if(encode(stream.read()))
			return(true);
	}
	return(false);
}

bool PMTK_GPS::wakeup() {
	DEBUGLN();
	DEBUGLN(F("Waking up"));
	stream.print('\n');
	// Wakeup command is not acked, so we accept any valid line as success
	return(readline());
}

void PMTK_GPS::coldStart() {
	wakeup();
	send(PMTK_CMD_COLD_START,false);
}

void PMTK_GPS::fullColdStart() {
	wakeup();
	send(PMTK_CMD_FULL_COLD_START,false);
}

bool PMTK_GPS::extendEphemerisTime(const unsigned int sv, const unsigned int snr, time_t extensionThreshold, time_t extension) {
	// Longest possible data fields:
	// 4,30,180000,3600000
	// 1234567890123456789
	char data[20]={};
	sprintf(data,"%d,%d,%lu,%lu",
		sv,
		snr,
		extensionThreshold,
		extension
	);
	return(send(PMTK_SET_AL_DEE_CFG,data));
}

bool PMTK_GPS::periodicMode(const unsigned int type, const uint32_t runTime, const uint32_t sleepTime, const uint32_t secondRunTime, const uint32_t secondSleepTime) {
	// Longest possible data fields:
	// 0,518400000,518400000,51840000,518400000
	// 1234567890123456789012345678901234567890
	char data[41]={};
	sprintf(data,"%d", type);
	if(runTime>0) {
		sprintf(&data[strlen(data)],",%lu",runTime);
		if(sleepTime>0) {
			sprintf(&data[strlen(data)],",%lu",sleepTime);
			if(secondRunTime>0) {
				sprintf(&data[strlen(data)],",%lu",secondRunTime);
				if(secondSleepTime>0) {
					sprintf(&data[strlen(data)],",%lu",secondSleepTime);
				}
			}
		}
	}
	return(send(PMTK_CMD_PERIODIC_MODE,data));
}

/*** Plumbing below ***/
bool PMTK_GPS::readline(const time_t timeout) {
	DEBUGLN("Readline was called");
	time_t start=millis();
	while(millis()-start<timeout) {
		if(loop()) {
			DEBUGLN();
			DEBUGLN(F("Parsed a GPS sentence"));
			return(true);
		}
	}
	DEBUGLN();
	DEBUGLN(F("Readline failed"));
	return(false);
}

unsigned int PMTK_GPS::checksum(const char* data, char parity) {
	for(size_t i(0); i<strlen(data); i++) {
		parity^=data[i];
	}
	return(parity);
}

bool PMTK_GPS::send(const unsigned int type, bool checkAck) {
	char c[]={'\0'};
	return(send(type,c,checkAck));
}

bool PMTK_GPS::send(const unsigned int type, const char* data, bool checkAck) {
	// Wake the module up first, in case a previous command has made it sleep
	if(!wakeup())
		return(false);
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
	// Empty incoming buffer
	loop();
	stream.print(buf);
	stream.flush();
	if(checkAck)
		return(getAck(type));
	else
		return(true);
}

bool PMTK_GPS::getAck(const unsigned int type) {
	// Clear ackType.isUpdated() and ackResponse.isUpdated()
	ackType.value();
	ackResponse.value();
	// The datasheet does not specify whether the response will be sent
	// immediately. We inspect a few lines before giving up
	int attempts=10;
	for(int i=attempts; i>0; i--) {
		if(readline()) {
			bool updated=(ackType.isUpdated()==1 && ackResponse.isUpdated()==1);
			unsigned long int gotType=atoi(ackType.value());
			unsigned long int gotResponse=atoi(ackResponse.value());
			DEBUGLN();
			DEBUG(F("Attempt: "));
			DEBUG(i);
			DEBUG(F("/"));
			DEBUGLN(attempts);
			DEBUG(F("Looking for response: "));
			DEBUGLN(PMTK_RESPONSE_SUCCESS);
			DEBUG(F("Looking for type: "));
			DEBUGLN(type);
			DEBUG(F("Was updated: "));
			DEBUGLN(updated);
			DEBUG(F("ackResponse.value()=="));
			DEBUGLN(gotType);
			DEBUG(F("ackType.value()=="));
			DEBUGLN(gotResponse);
			if(updated) {
				DEBUGLN();
				DEBUGLN(F("Updated"));
				if(gotType==type) {
					DEBUGLN();
					DEBUGLN(F("Type matched"));
					if(gotResponse==PMTK_RESPONSE_SUCCESS) {
						DEBUGLN();
						DEBUGLN(F("Correct response"));
						return(true);
					}
					else {
						DEBUGLN();
						DEBUGLN(F("Incorrect response"));
						return(false);
					}
				}
				else {
					DEBUGLN();
					DEBUGLN(F("Type did not match"));
				}
			}
			else {
				DEBUGLN();
				DEBUGLN(F("Not updated"));
			}
		}
		else {
			DEBUGLN();
			DEBUGLN(F("readline failed"));
		}
	}
	return(false);
}
