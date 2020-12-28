/*
 * Debug.h
 *
 *  Created on: 21.10.2016
 *      Author: maro
 */

#ifndef DEBUG_H_
#define DEBUG_H_



	String formatBytes(size_t bytes);//TODO: get this out of here
	void myprint(uint64_t value);

//#define DEBUG
#ifdef DEBUG
    #include <FS.h>
	#include "SPIFFS.h"
	#define DPRINT_INIT(x)	Serial.begin(x)
	#define DPRINT(x)  	Serial.print(x)
	#define DPRINTLN(x)  Serial.println(x)
	#define DPRINTH(x)  	Serial.print((x),HEX)
	#define DPRINTLNH(x)  Serial.println((x),HEX)
	#define DBGOUTPUT(x) Serial.setDebugOutput(x)
	#define PROJECT_NAME "NET1_LEDsDebug"
	#define ZVERSION "debug xxx " + String(__DATE__)
	#define ZUP_TIME " " + String(millis()/1000) + " [s]"
    #define SHOWSPIFFS   \
					SPIFFS.begin();\
					{\
						File dir = SPIFFS.open("/");\
						while (dir.openNextFile()) {\
							String fileName = dir.name();\
							size_t fileSize = dir.size();\
							Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());\
						}\
						Serial.printf("\n");\
					}
    #define DPRINT64(x)	myprint(x)



#else
	#define DPRINT_INIT(...)
	#define DPRINT(...)
	#define DPRINTLN(...)
	#define DPRINTH(...)
	#define DPRINTLNH(...)
    #define DBGOUTPUT(...)
	#define SHOWSPIFFS
	#define DPRINT64(...)
	#define PROJECT_NAME "NET1_LEDs"
	#define ZVERSION "xxxx" + String(__DATE__)
	#define ZUP_TIME " " + String(millis()/1000) + " [s]"
#endif

#endif /* DEBUG_H_ */
