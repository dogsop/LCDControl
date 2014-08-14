################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../JSON.o \
../JSONValue.o \
../LCDControl.o \
../LCDDisplay.o \
../PidController.o \
../SerialLCD.o 

CPP_SRCS += \
../JSON.cpp \
../JSONValue.cpp \
../LCDControl.cpp \
../LCDDisplay.cpp \
../PidController.cpp \
../SerialLCD.cpp 

OBJS += \
./JSON.o \
./JSONValue.o \
./LCDControl.o \
./LCDDisplay.o \
./PidController.o \
./SerialLCD.o 

CPP_DEPS += \
./JSON.d \
./JSONValue.d \
./LCDControl.d \
./LCDDisplay.d \
./PidController.d \
./SerialLCD.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


