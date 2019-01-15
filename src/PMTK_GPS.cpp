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
	// Clear ackedCommand.isUpdated() and ackResponse.isUpdated()
	ackedCommand.value();
	ackResponse.value();
	// The datasheet does not specify whether the response will be sent
	// immediately. We inspect a few lines before giving up
	int attempts=10;
	for(int i=attempts; i>0; i--) {
		if(readline(PMTK_READLINE_TIMEOUT)) {
			if(true) { // || ackedCommand.isUpdated() && ackResponse.isUpdated()) {
				//stream.print("ackResponse.isUpdated()==");
				//stream.println(ackResponse.isUpdated());
				//stream.print("ackedCommand.isUpdated()==");
				//stream.println(ackedCommand.isUpdated());
				//stream.print("ackResponse.value()==");
				//stream.println(atol(ackResponse.value()));
				//stream.print("ackedCommand.value()==");
				//stream.println(atol(ackedCommand.value()));
				if(atol(ackedCommand.value())==type) {
					//stream.println("Type matched");
					if(atol(ackResponse.value())==PMTK_CMD_SUCCESS)
						return(true);
					else
						return(false);
				}
				//else
			//		stream.println("Type did not match");
			}
		}
	}
	return(false);
}

void PMTK_GPS::wakeup() {
	stream.print('\n');
}

bool PMTK_GPS::reset() {
	wakeup();
	send(PMTK_CMD_FULL_COLD_START);
	//send(PMTK_SET_NMEA_BAUDRATE,0);
	// Wakeup command is not acked, so we accept any line as success
	return(readline());
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
