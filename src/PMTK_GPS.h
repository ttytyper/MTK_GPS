#include <Arduino.h>
#include <TinyGPS++.h>
#include <TimeLib.h>

class PMTK_GPS : private TinyGPSPlus {
	public:
		PMTK_GPS(Stream& stream);
		bool loop();
		void readline();
		bool readline(time_t timeout);
#define PMTK_PERIODIC_NORMAL 0
#define PMTK_PERIODIC_BACKUP 1
#define PMTK_PERIODIC_STANDBY 2
#define PMTK_PERPETUAL_BACKUP 4
#define PMTK_ALWAYSLOCATE_STANDBY 8
#define PMTK_ALWAYSLOCATE_BACKUP 9
		void periodicMode(unsigned int type,uint32_t runTime, uint32_t sleepTime, uint32_t secondRunTime=0, uint32_t secondSleepTime=0);
	private:
		Stream& stream;
		void send(unsigned int type, char* data);
		unsigned int checksum(char* data);
};
