/**
Arduino library for talking to MediaTek MT3318, MT3320 and MT3339 GPS receivers.
Copyright (C) 2019 Jesper Henriksen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MTK_GPS_H
#define MTK_GPS_H

// User can set MTK_DEBUG to e.g. Serial to get debug output
//#define MTK_DEBUG Serial

#ifdef MTK_DEBUG
#define DEBUG(...) MTK_DEBUG.print(__VA_ARGS__)
#define DEBUGLN(...) MTK_DEBUG.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGLN(...)
#endif

#include <Arduino.h>
#include <TinyGPS++.h>
#include <TimeLib.h>

#define MTK_READLINE_TIMEOUT 5000
#define MTK_TALKERID "PMTK"

// Timeout is issued when no data could be read from the GPS. This is not part
// of the GlobalTop datasheet
#define MTK_ACK_TIMEOUT -1
#define MTK_ACK_INVALID 0
#define MTK_ACK_UNSUPPORTED 1
#define MTK_ACK_FAIL 2
#define MTK_ACK_SUCCESS 3
typedef int mtk_ack_t;

/*** Command numbers ***/
#define MTK_CMD_HOT_START 101
#define MTK_CMD_WARM_START 101
#define MTK_CMD_COLD_START 103
#define MTK_CMD_FULL_COLD_START 104
#define MTK_SET_AL_DEE_CFG 223
#define MTK_CMD_PERIODIC_MODE 225
#define MTK_SET_NMEA_UPDATERATE 220
#define MTK_SET_NMEA_BAUDRATE 251

/*** Operation modes for MTK_CMD_PERIODIC_MODE ***/
#define MTK_PERIODIC_NORMAL 0
#define MTK_PERIODIC_BACKUP 1
#define MTK_PERIODIC_STANDBY 2
#define MTK_PERPETUAL_BACKUP 4
#define MTK_ALWAYSLOCATE_STANDBY 8
#define MTK_ALWAYSLOCATE_BACKUP 9

class MTK_GPS : public TinyGPSPlus {
	public:
		MTK_GPS(Stream& stream);
		bool loop();
		void wakeup();
		void hotStart();
		void warmStart();
		void coldStart();
		void fullColdStart();
		mtk_ack_t setNmeaUpdateRate(const unsigned int rate=1000);
		mtk_ack_t setNmeaBaudRate(const unsigned long int rate=0);
		mtk_ack_t extendEphemerisTime(
			const unsigned int sv=1,
			const unsigned int snr=30,
			const time_t extensionThreshold=180000,
			const time_t extension=60000);
		mtk_ack_t periodicMode(
			const unsigned int mode,
			const uint32_t runTime=0,
			const uint32_t sleepTime=0,
			const uint32_t secondRunTime=0,
			const uint32_t secondSleepTime=0);
	private:
		Stream& stream;
		TinyGPSCustom ackType;
		TinyGPSCustom ack;
		bool readline(const time_t timeout=MTK_READLINE_TIMEOUT);
		char checksum(const long unsigned int data);
		char checksum(const char* data);
		char sendFields();
		template<typename First, typename ... Next>
			char sendFields(const First& first, const Next& ... next);
		template<typename ... Param>
			int sendWithAck(const unsigned int type, const Param& ... param);
		template<typename ... Param>
			void send(const unsigned int type, const Param& ... param);
		mtk_ack_t getAck(const unsigned int type);
};

#endif
