/*
 * PidController.h
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#ifndef PIDCONTROLLER_H_
#define PIDCONTROLLER_H_

#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <pthread.h>

using namespace std;

#define MANUAL 0
#define AUTOMATIC 1

#define DIRECT 0
#define REVERSE 1

class PidController {
public:
	PidController();
	virtual ~PidController();
	double Compute();
	void SetControllerTunings(double Kp, double Ki, double Kd);
	void SetControllerSampleTime(int NewSampleTime);
	void SetControllerOutputLimits(double Min, double Max);
	void SetControllerMode(int Mode);
	void InitializeController();
	void SetControllerDirection(int Direction);
	void SetControllerSetpoint(double newSetpoint);
	bool GetControllerMode();
	const string buildPidTemperatureString(double inputTemp, double blowerOutput, double errorTerm, double derivativeError, double proportialOutputTerm, double integralOutputTerm, double derivativeOutputTerm);


private:
	/*working variables*/
	double inputTemp, blowerOutput, setPointTemp;
	double integralOutputTerm, lastInputTemp;
	double kp, ki, kd;
	int sampleTime; //1 sec
	double outMin;
	double outMax;
	bool inAuto;
	int controllerDirection;

};

#endif /* PIDCONTROLLER_H_ */
