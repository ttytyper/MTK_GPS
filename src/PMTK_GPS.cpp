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

void PMTK_GPS::wakeup() {
	DEBUGLN();
	DEBUGLN(F("Waking up"));
	stream.print('\n');
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
	send(PMTK_CMD_FULL_COLD_START);
}

// Valid range: 100-10000 (10 updates per second, to once per 10 seconds)
pmtk_ack_t PMTK_GPS::setNmeaUpdateRate(const unsigned int rate) {
	DEBUG(F("Setting update rate: "));
	DEBUGLN(rate);
	return(sendWithAck(PMTK_SET_NMEA_UPDATERATE,rate));
}

// Available baud rates: 4800, 9600, 14400, 19200, 38400, 57600, 115200
pmtk_ack_t PMTK_GPS::setNmeaBaudRate(const long unsigned int rate) {
	DEBUG(F("Setting baud rate: "));
	// It would appear that this command is not acked. Datasheet unclear
	//send(PMTK_SET_NMEA_BAUDRATE,buf,false);
	return(sendWithAck(PMTK_SET_NMEA_BAUDRATE,rate));
}

// Extend time for receiving ephemeris data
// Quote from the datasheet, which is a little hard to understand:
// sv: It means the module need extend the time to receive more ephemeris data while the number of satellite without ephemeris data. Default 1, range 1-4
// snr: It means the module needs to enable the ephemeris data receiving while the SNR of satellite is more than the setting value. Default 30, range 25-30
// extensionThreshold: Extension time for ephemeris data receiving in ms. Default 1800000, range 40000-1800000
// extension: Gap time between EPH data receiving. Default 60000, range 0-3600000
pmtk_ack_t PMTK_GPS::extendEphemerisTime(
		const unsigned int sv,
		const unsigned int snr,
		const time_t extensionThreshold,
		const time_t extension
	) {
	return(sendWithAck(
		PMTK_SET_AL_DEE_CFG,
		sv,
		snr,
		extensionThreshold,
		extension
	));
}

pmtk_ack_t PMTK_GPS::periodicMode(
		const unsigned int mode,
		const uint32_t runTime,
		const uint32_t sleepTime,
		const uint32_t secondRunTime,
		const uint32_t secondSleepTime
	) {
	return(sendWithAck(
		PMTK_CMD_PERIODIC_MODE,
		mode,
		runTime,
		sleepTime,
		secondRunTime,
		secondSleepTime
	));
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

char PMTK_GPS::checksum(const long unsigned int data) {
	char buf[sizeof(long unsigned int)*8+1]={};
	sprintf(buf,"%lu",data);
	return(checksum(buf));
}

// Calculates XOR parity of a char array
char PMTK_GPS::checksum(const char* data) {
	char parity=0;
	for(size_t i(0); i<strlen(data); i++) {
		parity^=data[i];
	}
	return(parity);
}

// Constructs and sends a part of an NMEA sentence with an arbitrary number of
// comma-separated fields of arbitrary types, to be used as parameters for GPS
// commands.
//
// Returns the XOR parity of the whole string, to be used for building the NMEA
// suffix.
template<typename First, typename ... Next>
char PMTK_GPS::sendFields(const First& first, const Next& ... next) {
	stream.print(',');
	stream.print(first);
	// Return XOR parity of separator and fields
	return(','
		^checksum(first)
		^sendFields(next...)
	);
}
// sendFields(...) above calls itself recursively. On the final call there are
// no more parameters left, and the empty sendFields() function below is called
// to end the recursion
char PMTK_GPS::sendFields() {
	return(0);
}

// Some GPS commands are acknowledged by the GPS. This function is like send()
// but additionally checks and returns the response from the GPS
template<typename ... Param>
int PMTK_GPS::sendWithAck(const unsigned int type, const Param& ... param) {
	// Flush incoming buffer so we don't have to potentially read a bunch of
	// NMEA sentences before getting to the ack
	loop();
	send(type,param...);

	// Clear ackType.isUpdated() and ack.isUpdated()
	ackType.value();
	ack.value();

	// The datasheet does not specify whether the ack will be sent
	// immediately. We inspect a few lines before giving up
	for(int retries=10; retries>0; retries--) {
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

// Sends a GPS command with an arbitrary number of parameter fields. Every
// command has a number (called type) specified in the datasheet for the GPS.
// These command numbers are listed as macros in the header file for readability.
template<typename ... Param>
void PMTK_GPS::send(const unsigned int type, const Param& ... param) {
	// Wake the module up first, in case a previous command has made it sleep
	wakeup();
	stream.print("$");
	stream.print(PMTK_TALKERID);
	stream.print(type);

	char suffix[6]; // Long enough for '*', XOR parity ("00") and linebreak ("\r\n"), plus null
	// Calculates the final XOR parity and constructs the NMEA suffix
	sprintf(suffix,"*%02X\r\n",
			checksum(PMTK_TALKERID)
			^checksum(type)
			^sendFields(param...)
	);
	stream.print(suffix);
	stream.flush();
}
// }}} Plumbing
