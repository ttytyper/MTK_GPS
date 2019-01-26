#include "PMTK_GPS.h"

PMTK_GPS::PMTK_GPS(Stream& stream):
	stream(stream),
	ackType(*this,"PMTK001",1),
	ack(*this,"PMTK001",2) {
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

// Hot restart: Use all available data in the NV store
void PMTK_GPS::hotStart() {
	// It would appear that this command is not acked. Datasheet unclear
	send(PMTK_CMD_HOT_START);
}

// Warm restart: Don't use ephemeris at restart
void PMTK_GPS::warmStart() {
	// It would appear that this command is not acked. Datasheet unclear
	send(PMTK_CMD_WARM_START);
}

// Don't use time, position, almanacs and ephemeris at restart
void PMTK_GPS::coldStart() {
	// It would appear that this command is not acked. Datasheet unclear
	send(PMTK_CMD_COLD_START);
}

// Perform cold restart and clear system/user configuration. In other words, reset to factory configuration
void PMTK_GPS::fullColdStart() {
	// It would appear that this command is not acked. Datasheet unclear
	send(PMTK_CMD_FULL_COLD_START,false);
}

// Extend time for receiving ephemeris data
// Quote from the datasheet, which is a little hard to understand:
// sv: It means the module need extend the time to receive more ephemeris data while the number of satellite without ephemeris data. Default 1, range 1-4
// snr: It means the module needs to enable the ephemeris data receiving while the SNR of satellite is more than the setting value. Default 30, range 25-30
// extensionThreshold: Extension time for ephemeris data receiving in ms. Default 1800000, range 40000-1800000
// extension: Gap time between EPH data receiving. Default 60000, range 0-3600000
int PMTK_GPS::extendEphemerisTime(const unsigned int sv, const unsigned int snr, time_t extensionThreshold, time_t extension) {
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

int PMTK_GPS::periodicMode(const unsigned int type, const uint32_t runTime, const uint32_t sleepTime, const uint32_t secondRunTime, const uint32_t secondSleepTime) {
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

// Plumbing {{{
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

int PMTK_GPS::send(const unsigned int type, bool checkAck) {
	char c[]={'\0'};
	return(send(type,c,checkAck));
}

int PMTK_GPS::send(const unsigned int type, const char* data, bool checkAck) {
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

int PMTK_GPS::getAck(const unsigned int type) {
	// Clear ackType.isUpdated() and ack.isUpdated()
	ackType.value();
	ack.value();
	// The datasheet does not specify whether the ack will be sent
	// immediately. We inspect a few lines before giving up
	int attempts=10;
	for(int i=attempts; i>0; i--) {
		if(readline()) {
			bool updated=(ackType.isUpdated()==1 && ack.isUpdated()==1);
			unsigned long int gotType=atoi(ackType.value());
			unsigned long int gotAck=atoi(ack.value());
			DEBUGLN();
			DEBUG(F("Attempt: "));
			DEBUG(i);
			DEBUG(F("/"));
			DEBUGLN(attempts);
			DEBUG(F("Looking for ack: "));
			DEBUGLN(PMTK_ACK_SUCCESS);
			DEBUG(F("Looking for type: "));
			DEBUGLN(type);
			DEBUG(F("Was updated: "));
			DEBUGLN(updated);
			DEBUG(F("ack.value()=="));
			DEBUGLN(gotType);
			DEBUG(F("ackType.value()=="));
			DEBUGLN(gotAck);
			if(updated) {
				DEBUGLN();
				DEBUGLN(F("Updated"));
				if(gotType==type) {
					return(gotAck);
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
	return(PMTK_ACK_TIMEOUT);
}
// }}} Plumbing
