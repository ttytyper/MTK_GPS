#include <Arduino.h>
#include <TinyGPS++.h>
#include <TimeLib.h>
#define PMTK_ACK_TIMEOUT 1000
#define PMTK_BUFFER_LEN 255
#define PMTK_TALKERID "PMTK"

class PMTK_GPS {
	public:
		PMTK_GPS(Stream& stream, TinyGPSPlus gps);
		bool loop();
		void readline();
		bool readline(time_t timeout);
		bool getAck(unsigned int type);
		bool send(unsigned int type);
		bool send(unsigned int type, char* data);
		unsigned int checksum(const char* data, char parity=0);
#define PMTK_PERIODIC_NORMAL 0
#define PMTK_PERIODIC_BACKUP 1
#define PMTK_PERIODIC_STANDBY 2
#define PMTK_PERPETUAL_BACKUP 4
#define PMTK_ALWAYSLOCATE_STANDBY 8
#define PMTK_ALWAYSLOCATE_BACKUP 9
		bool periodicMode(unsigned int type,uint32_t runTime, uint32_t sleepTime, uint32_t secondRunTime=0, uint32_t secondSleepTime=0);
		void wakeup();
		bool reset();
	private:
		Stream& stream;
		TinyGPSPlus gps;
		TinyGPSCustom ackResponse;
		TinyGPSCustom ackedCommand;
};
