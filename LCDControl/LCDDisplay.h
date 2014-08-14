/*
 * LCDDisplay.h
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#ifndef LCDDISPLAY_H_
#define LCDDISPLAY_H_

#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include "SerialLCD.h"

class LCDDisplay {
    int clockThreadHandle;
    pthread_t clockThreadId;
    bool runClockFlag;
    static void *runClockFunc(void *);
    void runClock();
    std::string currentDateTime();
	void updateClock(const char *timeStr);

	SerialLCD *serialLcdPtr;
	pthread_mutex_t mutex;

public:
	LCDDisplay();
	virtual ~LCDDisplay();
	bool setupDisplay();
	void updateCookTimer(const char *elapsedTime);
	void updateSmokerTemp(const int temp);
	void updateMeatTemp(const int temp);
	void updateNetworkStatus(bool connected);
	void updateBlowerSpeed(const int temp);
};

#endif /* LCDDISPLAY_H_ */
