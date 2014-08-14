/*
 * PidController.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#include "LCDControl.h"

#include "PidController.h"

extern int smokerTemperature;


PidController::PidController() {
	inputTemp = 0.0;
	lastInputTemp = 0.0;
	integralOutputTerm = 0;
	blowerOutput = 0.0;
	setPointTemp = 225.0;
	sampleTime = 30; //1 sec
	kp = 0.0;
	ki = 0.0;
	kd = 0.0;
	outMin = 0.0;
	outMax = 100.0;
	inAuto = false;
	controllerDirection = DIRECT;
}

PidController::~PidController() {
	// TODO Auto-generated destructor stub
}

double PidController::Compute()
{
	string jsonString;

	if(smokerTemperature > 0) {
		inputTemp = (double)smokerTemperature;
	}

	printf("Input = %f\n", inputTemp);

   /*Compute all the working error variables*/
   double errorTerm = setPointTemp - inputTemp;
	printf("error = %f\n", errorTerm);

   integralOutputTerm += (ki * errorTerm);
    printf("ITerm = %f\n", integralOutputTerm);

   if(integralOutputTerm > outMax) integralOutputTerm= outMax;
   else if(integralOutputTerm < outMin) integralOutputTerm= outMin;
   printf("integralTerm = %f\n", integralOutputTerm);

   double derivativeError = (inputTemp - lastInputTemp);
   printf("derivativeError = %f\n", derivativeError);

   double proportialOutputTerm = kp * errorTerm;
   double derivativeOutputTerm = kd * derivativeError;

   /*Compute PID Output*/
   blowerOutput = proportialOutputTerm + integralOutputTerm - derivativeOutputTerm;
   printf("blowerOutput = %f\n", blowerOutput);

   if(blowerOutput > outMax) blowerOutput = outMax;
   else if(blowerOutput < outMin) blowerOutput = outMin;

   /*Remember some variables for next time*/
   lastInputTemp = inputTemp;

	jsonString = buildPidTemperatureString(inputTemp, blowerOutput, errorTerm, derivativeError, proportialOutputTerm, integralOutputTerm, derivativeOutputTerm);

	sendData(jsonString.c_str(), PID_DATA_PORT);

   return blowerOutput;
}

void PidController::SetControllerTunings(double Kp, double Ki, double Kd)
{
   if (Kp<0 || Ki<0|| Kd<0) return;

  double SampleTimeInSec = (double)sampleTime;

   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;

   printf("kp = %f\n", kp);
   printf("ki = %f\n", ki);
   printf("kd = %f\n", kd);

  if(controllerDirection ==REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

void PidController::SetControllerSampleTime(int NewSampleTime)
{
   if (NewSampleTime > 0)
   {
      double ratio  = (double)NewSampleTime
                      / (double)sampleTime;
      ki *= ratio;
      kd /= ratio;
      sampleTime = (unsigned long)NewSampleTime;
   }
}

void PidController::SetControllerOutputLimits(double Min, double Max)
{
   if(Min > Max) return;
   outMin = Min;
   outMax = Max;

   if(blowerOutput > outMax) blowerOutput = outMax;
   else if(blowerOutput < outMin) blowerOutput = outMin;

   if(integralOutputTerm > outMax) integralOutputTerm= outMax;
   else if(integralOutputTerm < outMin) integralOutputTerm= outMin;
}

void PidController::SetControllerMode(int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
    if(newAuto == !inAuto)
    {  /*we just went from manual to auto*/
    	InitializeController();
    }
    inAuto = newAuto;
}

void PidController::SetControllerSetpoint(double newSetpoint)
{
	setPointTemp = newSetpoint;
}

void PidController::InitializeController()
{
   lastInputTemp = inputTemp;
   integralOutputTerm = 0;
}

void PidController::SetControllerDirection(int Direction)
{
   controllerDirection = Direction;
}

bool PidController::GetControllerMode()
{
	return inAuto;
}

const string PidController::buildPidTemperatureString(double inputTemp, double blowerOutput, double errorTerm, double derivativeError, double proportialOutputTerm, double integralOutputTerm, double derivativeOutputTerm)
{
	JSONObject root;

	// Adding a string
	root[L"InputTemp"] = new JSONValue((double)inputTemp);
	root[L"BlowerOutput"] = new JSONValue((double)blowerOutput);
	root[L"ErrorTerm"] = new JSONValue((double)errorTerm);
	root[L"DerivativeError"] = new JSONValue((double)derivativeError);
	root[L"ProportialOutputTerm"] = new JSONValue((double)proportialOutputTerm);
	root[L"IntegralOutputTerm"] = new JSONValue((double)integralOutputTerm);
	root[L"DerivativeOutputTerm"] = new JSONValue((double)derivativeOutputTerm);

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

