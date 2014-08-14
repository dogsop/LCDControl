/*
 * SerialLCD.h
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#ifndef SERIALLCD_H_
#define SERIALLCD_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringSerial.h>


#define PAD_SPACE	0
#define PAD_ZERO	1

typedef enum {
	BOOTSCREEN_OFF = 0,
	BOOTSCREEN_CONFIG,
	BOOTSCREEN_USER
} SerialLCD_BootScreen_t;

typedef enum {
	CURSOR_OFF = 0,
	CURSOR_BLINKING,
	CURSOR_UNDERLINE,
	CURSOR_BOTH
} SerialLCD_Cursor_t;


class SerialLCD {
public:
	SerialLCD();
	virtual ~SerialLCD();
	bool begin( uint8_t w = 16, uint8_t h = 2 );
    void close();
    void clear();
	void clearline();
	void crlf();
    void home();
	void backward();
	void forward();
	void up();
	void down();
    void gotoxy( uint8_t, uint8_t );
    void scrollLeft();
    void scrollRight();
    void on();
    void off();
	void cursor(SerialLCD_Cursor_t c = CURSOR_OFF);
	void bootscreen(SerialLCD_BootScreen_t screen = BOOTSCREEN_OFF);
    void backlight(uint8_t);
    void putString(const char *string);
    void putChar(const char c);
	void command(uint8_t);
	//size_t puti(unsigned long n, uint8_t decimals, uint8_t base = 10, uint8_t frac = 0, uint8_t pad = PAD_ZERO);
	//void defineChar_P(uint8_t location, const uint8_t charmap[]);

private:
	int portHandle;
	int width;
	int height;

	pthread_mutex_t mutex;

};

#endif /* SERIALLCD_H_ */
