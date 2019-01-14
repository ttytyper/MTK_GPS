#include <Arduino.h>
#include <TinyGPS++.h>
#include <TimeLib.h>
#define PMTK_READLINE_TIMEOUT 1000
#define PMTK_TALKERID "PMTK"

#define PMTK_CMD_INVALID 0
#define PMTK_CMD_UNSUPPORTED 1
#define PMTK_CMD_FAIL 2
#define PMTK_CMD_SUCCESS 3

class PMTK_GPS : public TinyGPSPlus {
	public:
		PMTK_GPS(Stream& stream);
		bool loop();
		bool readline(const time_t timeout=PMTK_READLINE_TIMEOUT);
		bool getAck(const unsigned int type);
		bool send(const unsigned int type);
		bool send(const unsigned int type, const char* data);
		unsigned int checksum(const char* data, char parity=0);
#define PMTK_PERIODIC_NORMAL 0
#define PMTK_PERIODIC_BACKUP 1
#define PMTK_PERIODIC_STANDBY 2
#define PMTK_PERPETUAL_BACKUP 4
#define PMTK_ALWAYSLOCATE_STANDBY 8
#define PMTK_ALWAYSLOCATE_BACKUP 9
		bool periodicMode(const unsigned int type, const uint32_t runTime=0, const uint32_t sleepTime=0, const uint32_t secondRunTime=0, const uint32_t secondSleepTime=0);
		void wakeup();
		bool reset();
	private:
		Stream& stream;
		TinyGPSCustom ackedCommand;
		TinyGPSCustom ackResponse;
};
