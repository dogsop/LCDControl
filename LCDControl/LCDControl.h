/*
 * LCDControl.h
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#ifndef LCDCONTROL_H_
#define LCDCONTROL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <softPwm.h>

#include "JSON.h"
#include "SerialLCD.h"
#include "LCDDisplay.h"
#include "PidController.h"

using namespace std;

#define SRV_IP "127.0.0.1"
#define BUFLEN 512
#define NPACK 10
#define TEMP_DATA_PORT 9930
#define PID_DATA_PORT 9931


int readTemperature(int spiChannel);
const char *byte_to_binary(int x);
bool testNetworkConnection();
int sendData(const char *dataString, int udpPort);
bool readSetPointFile();
bool readPIDSettingsFile();

void *runNetworkMonitor(void *);
void *runTemperatureMonitor(void *);
void *runTemperatureController(void *);

void setBlower(int speed);

const string buildJsonTemperatureString();





#endif /* LCDCONTROL_H_ */
