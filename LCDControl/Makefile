CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		LCDControl.o JSON.o JSONValue.o LCDDisplay.o PidController.o SerialLCD.o

LIBS =		-lwiringPi -lpthread

TARGET =	LCDControl

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
