#ifndef PMTK_GPS_H
#define PMTK_GPS_H

// User can set PMTK_DEBUG to e.g. Serial to get debug output
//#define PMTK_DEBUG Serial

#ifdef PMTK_DEBUG
#define DEBUG(...) PMTK_DEBUG.print(__VA_ARGS__)
#define DEBUGLN(...) PMTK_DEBUG.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGLN(...)
#endif

#include <Arduino.h>
#include <TinyGPS++.h>
#include <TimeLib.h>

#define PMTK_READLINE_TIMEOUT 5000
#define PMTK_TALKERID "PMTK"

// Timeout is issued when no data could be read from the GPS. This is not part
// of the GlobalTop datasheet
#define PMTK_ACK_TIMEOUT -1
#define PMTK_ACK_INVALID 0
#define PMTK_ACK_UNSUPPORTED 1
#define PMTK_ACK_FAIL 2
#define PMTK_ACK_SUCCESS 3
typedef int pmtk_ack_t;

/*** Command numbers ***/
#define PMTK_CMD_HOT_START 101
#define PMTK_CMD_WARM_START 101
#define PMTK_CMD_COLD_START 103
#define PMTK_CMD_FULL_COLD_START 104
#define PMTK_SET_AL_DEE_CFG 223
#define PMTK_CMD_PERIODIC_MODE 225
#define PMTK_SET_NMEA_UPDATERATE 220
#define PMTK_SET_NMEA_BAUDRATE 251

/*** Operation modes for PMTK_CMD_PERIODIC_MODE ***/
#define PMTK_PERIODIC_NORMAL 0
#define PMTK_PERIODIC_BACKUP 1
#define PMTK_PERIODIC_STANDBY 2
#define PMTK_PERPETUAL_BACKUP 4
#define PMTK_ALWAYSLOCATE_STANDBY 8
#define PMTK_ALWAYSLOCATE_BACKUP 9

class PMTK_GPS : public TinyGPSPlus {
	public:
		PMTK_GPS(Stream& stream);
		bool loop();
		void wakeup();
		void hotStart();
		void warmStart();
		void coldStart();
		void fullColdStart();
		pmtk_ack_t setNmeaUpdateRate(const unsigned int rate=1000);
		pmtk_ack_t setNmeaBaudRate(const unsigned long int rate=0);
		pmtk_ack_t extendEphemerisTime(
			const unsigned int sv=1,
			const unsigned int snr=30,
			const time_t extensionThreshold=180000,
			const time_t extension=60000);
		pmtk_ack_t periodicMode(
			const unsigned int mode,
			const uint32_t runTime=0,
			const uint32_t sleepTime=0,
			const uint32_t secondRunTime=0,
			const uint32_t secondSleepTime=0);
	private:
		Stream& stream;
		TinyGPSCustom ackType;
		TinyGPSCustom ack;
		bool readline(const time_t timeout=PMTK_READLINE_TIMEOUT);
		char checksum(const long unsigned int data);
		char checksum(const char* data);
		char sendFields();
		template<typename First, typename ... Next>
			char sendFields(const First& first, const Next& ... next);
		template<typename ... Param>
			int sendWithAck(const unsigned int type, const Param& ... param);
		template<typename ... Param>
			void send(const unsigned int type, const Param& ... param);
		pmtk_ack_t getAck(const unsigned int type);
};

#endif
