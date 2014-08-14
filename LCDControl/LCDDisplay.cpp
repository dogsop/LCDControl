/*
 * LCDDisplay.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#include "LCDDisplay.h"

using namespace std;

LCDDisplay::LCDDisplay() {
	runClockFlag = true;
	clockThreadId = -1;
	clockThreadHandle = -1;
	serialLcdPtr = NULL;
	mutex = PTHREAD_MUTEX_INITIALIZER;
}

LCDDisplay::~LCDDisplay() {
	runClockFlag = false;

	pthread_mutex_lock( &mutex );
	serialLcdPtr->clear();
	serialLcdPtr->gotoxy(5,1);
	serialLcdPtr->putString("Shut down");
	pthread_mutex_unlock( &mutex );

	if(clockThreadId >= 0) {
		pthread_join( clockThreadId, NULL);
	}

	if(serialLcdPtr != NULL) {
		serialLcdPtr->close();
	}
}

bool LCDDisplay::setupDisplay() {

	if(serialLcdPtr != NULL) {
		return false;
	}

	serialLcdPtr = new SerialLCD();

	printf("Opening port\n");
	if(serialLcdPtr->begin(20,4) == false) {
		printf("unable to open LCD\n");
		return false;
	}

	printf("Locking mutex\n");
	pthread_mutex_lock( &mutex );
	serialLcdPtr->clear();
	serialLcdPtr->gotoxy(5,1);
	serialLcdPtr->putString("Smoker Monitor");
	printf("Unlocking mutex\n");
	pthread_mutex_unlock( &mutex );

	sleep(2);

	serialLcdPtr->clear();
	printf("Setting clock\n");
	updateClock("00:00:00");
	printf("Setting cook timer\n");
	updateCookTimer("00:00:00");
	printf("Setting smoker temp\n");
	updateSmokerTemp(-1);
	printf("Setting meat temp\n");
	updateMeatTemp(-1);

	sleep(2);


	printf("starting clock thread\n");
	clockThreadHandle = pthread_create(&clockThreadId, NULL, &runClockFunc, (void*)this);

	return true;
}

void *LCDDisplay::runClockFunc(void *self)
{
    ((LCDDisplay*)self)->runClock();
    return NULL;
}

void LCDDisplay::runClock()
{
	std::string timeString = currentDateTime();

	printf("in clock thread\n");

	while(runClockFlag == true) {
		std::string newTimeString = currentDateTime();
		if(timeString != newTimeString) {
			updateClock(timeString.c_str());
			timeString = newTimeString;
			sleep(1);
		}
	}
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string LCDDisplay::currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    //strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

void LCDDisplay::updateClock(const char *timeStr)
{
	pthread_mutex_lock( &mutex );
	serialLcdPtr->gotoxy(0,3);
	serialLcdPtr->putString(timeStr);
	pthread_mutex_unlock( &mutex );
}

void LCDDisplay::updateCookTimer(const char *elapsedTime)
{
	pthread_mutex_lock( &mutex );
	serialLcdPtr->gotoxy(12,3);
	serialLcdPtr->putString(elapsedTime);
	pthread_mutex_unlock( &mutex );
}

void LCDDisplay::updateSmokerTemp(const int temp)
{
	char tmpBuf[20];
	if(temp < 0) {
		strcpy(tmpBuf, "Smoker ----");
	} else {
		sprintf(tmpBuf, "Smoker %3d F", temp);
	}

	pthread_mutex_lock( &mutex );
	serialLcdPtr->gotoxy(0,0);
	serialLcdPtr->putString(tmpBuf);
	pthread_mutex_unlock( &mutex );
}

void LCDDisplay::updateMeatTemp(const int temp)
{
	char tmpBuf[20];
	if(temp < 0) {
		strcpy(tmpBuf, "Meat   ----");
	} else {
		sprintf(tmpBuf, "Meat   %3d F", temp);
	}

	pthread_mutex_lock( &mutex );
	serialLcdPtr->gotoxy(0,1);
	serialLcdPtr->putString(tmpBuf);
	pthread_mutex_unlock( &mutex );
}

void LCDDisplay::updateBlowerSpeed(const int blowerSpeed)
{
	char tmpBuf[20];
	if(blowerSpeed < 0 || blowerSpeed > 100) {
		strcpy(tmpBuf, "Blower ----");
	} else {
		sprintf(tmpBuf, "Blower %3d%%", blowerSpeed);
	}

	pthread_mutex_lock( &mutex );
	serialLcdPtr->gotoxy(0,2);
	serialLcdPtr->putString(tmpBuf);
	pthread_mutex_unlock( &mutex );
}

void LCDDisplay::updateNetworkStatus(bool connected)
{
	pthread_mutex_lock( &mutex );
	serialLcdPtr->gotoxy(15,0);
	serialLcdPtr->putString("     ");
	serialLcdPtr->gotoxy(15,0);
	if(connected == true) {
		serialLcdPtr->putString("   OK");
	} else {
		serialLcdPtr->putString("ERROR");
	}
	pthread_mutex_unlock( &mutex );
}


