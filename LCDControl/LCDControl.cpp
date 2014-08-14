//============================================================================
// Name        : LCDControl.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
#include "LCDControl.h"

pthread_t networkThreadId;
pthread_t temperatureThreadId;
pthread_t controllerThreadId;

int networkThreadHandle;
int temperatureThreadHandle;
int controllerThreadHandle;

LCDDisplay *LcdDisplayPtr;

PidController *PidControllerPtr;

bool timerRunningFlag = false;
time_t cookTimer;

bool runNetworkFlag = true;
bool runTemperatureFlag = true;
bool runControllerFlag = true;

int meatTemperature;
int smokerTemperature;

bool pwmInitialized = false;
int currentBlowerSpeed = 0;

double Kp;
double Ki;
double Kd;
double setPointTemp;
bool controllerRunning = false;

void ctrlc_handler(int s)
{
	printf("Caught signal %d\n",s);
	runNetworkFlag = false;
	runTemperatureFlag = false;
	runControllerFlag = false;
}

void usr1_handler(int s)
{
	printf("Caught signal %d\n",s);
	if(readSetPointFile() != true) {
		printf("readSetPointFile failed\n");
	} else {
		PidControllerPtr->SetControllerSetpoint(setPointTemp);
		if(controllerRunning == true) {
			PidControllerPtr->SetControllerMode(1);
		} else {
			PidControllerPtr->SetControllerMode(0);
			setBlower(0);
		}
	}

	if(readPIDSettingsFile() != true) {
		printf("readPIDSettingsFile failed\n");
	} else {
		PidControllerPtr->SetControllerTunings(Kp, Ki, Kd);
	}

}


int main(void)
{

	//int Counter;

	wiringPiSetup();
	PidControllerPtr = new PidController();

	if(readSetPointFile() != true) {
		printf("readSetPointFile failed\n");
		exit(-1);
	}

	if(readPIDSettingsFile() != true) {
		printf("readPIDSettingsFile failed\n");
		exit(-1);
	}

	printf("testing network\n");

	if(testNetworkConnection() == false) {
		printf("no network\n");
		return -1;
	}

	LcdDisplayPtr = new LCDDisplay;

	LcdDisplayPtr->setupDisplay();

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = ctrlc_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	struct sigaction sigUsr1Handler;

	sigUsr1Handler.sa_handler = usr1_handler;
	sigemptyset(&sigUsr1Handler.sa_mask);
	sigUsr1Handler.sa_flags = 0;

	sigaction(SIGUSR1, &sigUsr1Handler, NULL);


	printf("starting network thread\n");
	networkThreadHandle = pthread_create(&networkThreadId, NULL, &runNetworkMonitor, NULL);

	printf("starting temperature thread\n");
	temperatureThreadHandle = pthread_create(&temperatureThreadId, NULL, &runTemperatureMonitor, NULL);

	printf("starting controller thread\n");
	controllerThreadHandle = pthread_create(&controllerThreadId, NULL, &runTemperatureController, NULL);

	if( softPwmCreate(1,0,100) == 0) {
		printf("PWM init\n");
		pwmInitialized = true;
	} else {
		printf("PWM init failed\n");
	}

	pthread_join( networkThreadId, NULL);
	pthread_join( temperatureThreadId, NULL);
	pthread_join( controllerThreadId, NULL);

	delete LcdDisplayPtr;

	return EXIT_SUCCESS;
}

void setBlower(int speed)
{
	printf("softPwmWrite(1, %d)\n", speed);
	if(speed != currentBlowerSpeed) {
		if(currentBlowerSpeed == 0 && speed < 50) {
			softPwmWrite(1, 100);
			sleep(10);
		}
		currentBlowerSpeed = speed;
		LcdDisplayPtr->updateBlowerSpeed(currentBlowerSpeed);
		softPwmWrite(1, speed);
	}
}

int readTemperature(int spiChannel)
{
	int returnValue = -1;

	int fileHandle = wiringPiSPISetup(spiChannel, 500000);

	if(fileHandle < 0) {
		printf("wiringPiSPISetup failed\n");
		return -1;
	}

	unsigned char buffer[4];

	memset((void *)buffer, 0, 4);

	//printf("Reading\n");
	//printf("\n");

	wiringPiSPIDataRW(spiChannel, buffer, 4);

	close(fileHandle);

	int Counter;

	for(Counter = 0; Counter < 4; Counter++) {
		//printf("buffer[%d] = %02X\n", Counter, buffer[Counter]);
	}

	for(Counter = 0; Counter < 4; Counter++) {
		//printf("%s ", byte_to_binary(buffer[Counter]));
	}
	//printf("\n");
	//printf("\n");

	int faultBit = buffer[1] & 0x01;
	//printf("Fault bit = %d\n", faultBit);

	if(faultBit) {
		int scvFault = 0;
		if((buffer[3] & 0x04) > 0) {
			scvFault = 1;
		}
		fprintf(stderr, "scv Fault = %d\n", scvFault);

		int scgFault = 0;
		if((buffer[3] & 0x02) > 0) {
			scgFault = 1;
		}
		fprintf(stderr, "scg Fault = %d\n", scgFault);

		int ocFault = 0;
		if((buffer[3] & 0x01) > 0) {
			ocFault = 1;
		}
		fprintf(stderr, "oc Fault = %d\n", ocFault);
		//printf("\n");
		return -1;
	}

	int intTemp = 0;
	int Sign = 1;
	unsigned char bitMask = 0x80;

	for(Counter = 0; Counter < 8; Counter++) {
		if(buffer[0] & bitMask) {
			if(Counter == 0) {
				Sign = -1;
			} else {
				intTemp += 1;
			}
		}
		intTemp *= 2;
		bitMask >>= 1;
	}

	bitMask = 0x80;

	for(Counter = 0; Counter < 6; Counter++) {
		if(buffer[1] & bitMask) {
			intTemp += 1;
		}
		if(Counter < 5) {
			intTemp *= 2;
		}
		bitMask >>= 1;
	}
	////printf("Integer Temp = %d\n", intTemp);

	double temperature = (double)intTemp/(double)4;
	temperature *= (double)Sign;

	//printf("Temp = %0.2lf C\n", temperature);
	temperature = temperature * 9.0/5.0 + 32.0;
	//printf("Temp = %0.3lf F\n", temperature);
	//printf("\n");
	returnValue = (int)temperature;

	intTemp = 0;
	Sign = 1;
	bitMask = 0x80;

	for(Counter = 0; Counter < 8; Counter++) {
		if(buffer[2] & bitMask) {
			if(Counter == 0) {
				Sign = -1;
			} else {
				intTemp += 1;
			}
		}
		intTemp *= 2;
		bitMask >>= 1;
	}

	bitMask = 0x80;

	for(Counter = 0; Counter < 4; Counter++) {
		if(buffer[3] & bitMask) {
			intTemp += 1;
		}
		if(Counter < 3) {
			intTemp *= 2;
		}
		bitMask >>= 1;
	}
	////printf("Integer Temp = %d\n", intTemp);

	temperature = (double)intTemp/(double)16;
	temperature *= (double)Sign;

	//printf("internal Temp = %0.2lf C\n", temperature);
	temperature = temperature * 9.0/5.0 + 32.0;
	//printf("internal Temp = %0.3lf F\n", temperature);
	//printf("\n");

	return returnValue;

}

const char *byte_to_binary(int x)
{
	static char b[9];
	b[0] = '\0';

	int z;
	for(z = 128; z > 0; z >>= 1) {
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}


bool testNetworkConnection()
{
        struct addrinfo hints, *res, *p;
        int status, sockfd;
        char ipstr[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo("www.google.com", "http", &hints, &res)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                return false;
        }

        //printf("Connection to...");

        for(p = res;p != NULL; p = p->ai_next) {
                void *addr;
                //const char *ipver;

                // get the pointer to the address itself,
                // different fields in IPv4 and IPv6:
                if (p->ai_family == AF_INET) { // IPv4
                        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                        addr = &(ipv4->sin_addr);
                        //ipver = "IPv4";
                } else { // IPv6
                        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                        addr = &(ipv6->sin6_addr);
                        //ipver = "IPv6";
                }

                // convert the IP to a string and print it:
                inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
                //printf("  %s: %s\n", ipver, ipstr);
        }

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        	//printf("Connection error\n");
                return false;
        } else {
        	//printf("Connection successful\n");
        }

        close(sockfd);

        freeaddrinfo(res); // free the linked list

        return true;
}

void *runNetworkMonitor(void *)
{
	printf("in network thread\n");

	while(runNetworkFlag == true) {
		LcdDisplayPtr->updateNetworkStatus(testNetworkConnection());
		sleep(15);
	}
    return NULL;
}

void *runTemperatureController(void *)
{
	printf("in controller thread\n");

	PidControllerPtr = new PidController();

	PidControllerPtr->InitializeController();
	PidControllerPtr->SetControllerSetpoint(setPointTemp);
	PidControllerPtr->SetControllerTunings(Kp, Ki, Kd);
	if(controllerRunning == true) {
		PidControllerPtr->SetControllerMode(1);
	} else {
		PidControllerPtr->SetControllerMode(0);
		setBlower(0);
	}

	sleep(30);

	while(runControllerFlag == true) {
		if(PidControllerPtr->GetControllerMode() == true) {
			int newBlowerSpeed;
			printf("calling PidControllerPtr->Compute()\n");
			newBlowerSpeed = (int)PidControllerPtr->Compute();
			printf("PidControllerPtr->Compute() returned %d\n",newBlowerSpeed);
			setBlower(newBlowerSpeed);
		} else {
			printf("controller mode false\n");
		}
		sleep(30);
	}
	setBlower(0);
    return NULL;
}

void *runTemperatureMonitor(void *)
{
	string jsonString;

	printf("in temperature thread\n");

	while(runTemperatureFlag == true) {
		meatTemperature = readTemperature(0);
		smokerTemperature = readTemperature(1);

		LcdDisplayPtr->updateMeatTemp(meatTemperature);
		LcdDisplayPtr->updateSmokerTemp(smokerTemperature);

		jsonString = buildJsonTemperatureString();

		sendData(jsonString.c_str(), TEMP_DATA_PORT);

		sleep(10);
	}
    return NULL;
}

const string buildJsonTemperatureString()
{
	JSONObject root;

	// Adding a string
	root[L"MeatTemp"] = new JSONValue((double)meatTemperature);
	root[L"SmokerTemp"] = new JSONValue((double)smokerTemperature);

	root[L"BlowerSetting"] = new JSONValue((double)currentBlowerSpeed);

	// Create a value
	JSONValue *value = new JSONValue(root);

	wstring wtest = value->Stringify();
	string test(wtest.begin(), wtest.end());

	// Print it
	//printf("%s\n", test.c_str());


	// Clean up
	delete value;

	return test;
}

bool readSetPointFile()
{
	bool returnValue = false;
	FILE *inputFile;

	printf("readSetPointFile\n");

	inputFile = fopen("/home/pi/SmokerController/setPoint.json", "r");
	if(inputFile != NULL) {
		char inputString[1024];
		if(fgets(inputString, 1024, inputFile) != NULL) {
			printf("input - %s\n", inputString);
			printf("calling Parse\n");
			JSONValue *v = JSON::Parse(inputString);
			if(v != NULL) {
				printf("Parse worked\n");
				if(v->IsObject() == true) {
					printf("v is object\n");
					JSONObject root;
					root = v->AsObject();
					if (root.find(L"SetPointTemp") != root.end() && root[L"SetPointTemp"]->IsNumber()) {
					//if (root.find(L"Kp") != root.end()) {
						printf("found SetPointTemp\n");
						setPointTemp = root[L"SetPointTemp"]->AsNumber();
						printf("SetPointTemp = %lf\n", setPointTemp);
					}
					if (root.find(L"ControllerRunning") != root.end() && root[L"ControllerRunning"]->IsBool()) {
						printf("found ControllerRunning\n");
						controllerRunning = root[L"ControllerRunning"]->AsBool();
					}
					returnValue = true;
				} else {
					printf("v is not object\n");
				}
			} else {
				printf("Parse returned NULL\n");
			}
		} else {
			printf("read failed\n");
		}
	} else {
		printf("input file not found\n");
	}
	return returnValue;
}

bool readPIDSettingsFile()
{
	FILE *inputFile;
	bool returnValue = false;

	printf("readSetPointFile\n");

	inputFile = fopen("/home/pi/SmokerController/pidSettings.json", "r");
	if(inputFile != NULL) {
		char inputString[1024];
		if(fgets(inputString, 1024, inputFile) != NULL) {
			printf("input - %s\n", inputString);
			printf("calling Parse\n");
			JSONValue *v = JSON::Parse(inputString);
			if(v != NULL) {
				printf("Parse worked\n");
				if(v->IsObject() == true) {
					printf("v is object\n");
					JSONObject root;
					root = v->AsObject();
					if (root.find(L"Kp") != root.end() && root[L"Kp"]->IsNumber()) {
					//if (root.find(L"Kp") != root.end()) {
						printf("found Kp\n");
						Kp = root[L"Kp"]->AsNumber();
						printf("Kp = %lf\n", Kp);
					}
					if (root.find(L"Ki") != root.end() && root[L"Ki"]->IsNumber()) {
						printf("found Ki\n");
						Ki = root[L"Ki"]->AsNumber();
						printf("Ki = %lf\n", Ki);
					}
					if (root.find(L"Kd") != root.end() && root[L"Kd"]->IsNumber()) {
						printf("found Kd\n");
						Kd = root[L"Kd"]->AsNumber();
						printf("Kd = %lf\n", Kd);
					}
					returnValue = true;
				} else {
					printf("v is not object\n");
				}
			} else {
				printf("Parse returned NULL\n");
			}
		} else {
			printf("read failed\n");
		}
	} else {
		printf("input file not found\n");
	}
	return returnValue;
}

int sendData(const char *dataString, int udpPort) {
	struct sockaddr_in si_other;
	int s;
	socklen_t slen = sizeof(si_other);

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		fprintf(stderr, "socket() failed\n");
		return -1;
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(udpPort);
	if (inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		return -1;
	}

	printf("Sending #%s#\n", dataString);

	int bytesSent;

	if ((bytesSent = sendto(s, (const void *)dataString, strlen(dataString), 0, (const sockaddr *)&si_other, slen)) == -1) {
		fprintf(stderr, "socket() failed\n");
		return -1;
	}

	printf("bytes sent %d\n", bytesSent);

	close(s);
	return 0;
}
