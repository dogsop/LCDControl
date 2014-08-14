/*
 * SerialLCD.cpp
 *
 *  Created on: Jul 13, 2014
 *      Author: ken
 */

#include "SerialLCD.h"

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

SerialLCD::SerialLCD() {
	portHandle = -1;
	width = 20;
	height = 2;
	mutex = PTHREAD_MUTEX_INITIALIZER;
}

SerialLCD::~SerialLCD() {
	// TODO Auto-generated destructor stub
}

// Initialize the Serial LCD Driver.
bool SerialLCD::begin(uint8_t w, uint8_t h)
{
	portHandle = serialOpen("/dev/ttyAMA0", 19200);

	if(portHandle < 0) {
		//printf("unable to open port\n");
		return false;
	}

	width = w;
	height = h;

	// Configure LCD geometry (2x16,2x20,2x24,2x40,4x16,4x20)
	// Clip the parameters to supported values
	uint8_t _w = MIN( 40, MAX( 16, width ) );
	uint8_t _h = MIN(  4, MAX( 2, height ) );
	serialPrintf(portHandle, "?G%d%d", _h, _w);

	delay(300);	// pause to allow LCD EEPROM to program
	serialPuts(portHandle, "?s4"); // set tab to 4 spaces
	delay(300); // pause to allow LCD EEPROM to program

	// this delay is needed for lcd init
    delay(600);

	return true;
}

void SerialLCD::close()
{
	if(portHandle < 0) {
		return;
	}
	serialClose(portHandle);
}

// Clear the display
void SerialLCD::clear()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?f");
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Clear cursor line
void SerialLCD::clearline()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?l");
	delay(10);
	pthread_mutex_unlock( &mutex );
}

void SerialLCD::crlf()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?n");
	pthread_mutex_unlock( &mutex );
}

// Return to home(top-left corner of LCD)
void SerialLCD::home()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?a");
	pthread_mutex_unlock( &mutex );
}

// backward cursor
void SerialLCD::backward()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?h");
	pthread_mutex_unlock( &mutex );
}

// forward cursor
void SerialLCD::forward()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?i");
	pthread_mutex_unlock( &mutex );
}

// up cursor
void SerialLCD::up()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?j");
	pthread_mutex_unlock( &mutex );
}

// down cursor
void SerialLCD::down()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?k");
	pthread_mutex_unlock( &mutex );
}

// Set Cursor to (Column,Row) Position
void SerialLCD::gotoxy(uint8_t col, uint8_t row)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPrintf(portHandle, "?x%02d?y%d", col, row);
	pthread_mutex_unlock( &mutex );
}

// Set the bootscreen 0 - off, 1 - config, 2 - user
void SerialLCD::bootscreen(SerialLCD_BootScreen_t b)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	switch(b) {
		default:
		case BOOTSCREEN_OFF: // off
			serialPuts(portHandle, "?S0");
		break;
		case BOOTSCREEN_CONFIG: // config screen
			serialPuts(portHandle, "?S1");
		break;
		case BOOTSCREEN_USER: // user defined
			serialPuts(portHandle, "?S2");
		break;
	}
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Switch the display on without clearing RAM
void SerialLCD::on()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?!0C"); // display on
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Switch the display off without clearing RAM
void SerialLCD::off()
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?!08"); // display off
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Set the cursor 0 - off,
void SerialLCD::cursor(SerialLCD_Cursor_t c)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	switch(c) {
		default:
		case CURSOR_OFF: // cursor is off
			serialPuts(portHandle, "?c0");
		break;
		case CURSOR_BLINKING: // cursor is blinking
			serialPuts(portHandle, "?c1");
		break;
		case CURSOR_UNDERLINE: // cursor is underline
			serialPuts(portHandle, "?c2");
		break;
		case CURSOR_BOTH: // both cursors active
			serialPuts(portHandle, "?c3");
		break;
	}
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Scroll the display left without changing the RAM
void SerialLCD::scrollLeft(void)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?!18");
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Scroll the display right without changing the RAM
void SerialLCD::scrollRight(void)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, "?!1E");
	delay(10);
	pthread_mutex_unlock( &mutex );
}

// Set the backlight 0 - off, 0xff - max
void SerialLCD::backlight(uint8_t thePercentage)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	uint8_t theValue = (thePercentage * 255)/100;

	serialPrintf(portHandle, "?B%02X", theValue);
    delay(10);
	pthread_mutex_unlock( &mutex );
}

void SerialLCD::command(uint8_t cmd)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPrintf(portHandle, "?!%02X", cmd);
	delay(10);
	pthread_mutex_unlock( &mutex );
}

void SerialLCD::putString(const char *string)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPuts(portHandle, string);
	pthread_mutex_unlock( &mutex );
}

void SerialLCD::putChar(const char c)
{
	if(portHandle < 0) {
		return;
	}

	pthread_mutex_lock( &mutex );
	serialPutchar(portHandle, c);
	pthread_mutex_unlock( &mutex );
}

//size_t SerialLCD::puti(unsigned long n, uint8_t decimals, uint8_t base, uint8_t frac, uint8_t pad)
//{
//	if(portHandle < 0) {
//		return;
//	}
//
//	uint8_t digits = decimals + frac;
//	uint8_t len = digits + (frac?1:0); // add one for '.'
//	char buf[len+1]; // Assumes len chars plus zero byte.
//	char *str = &buf[len];
//
//	*str = '\0';
//	// prevent crash if called with base == 1
//	if (base < 2) base = 10;
//
//	while (digits-- > 0) {
//		if(n){
//			unsigned long m = n;
//			// mod = x - ((x / z) * z)
//			n /= base;
//			char c = m - base * n;
//			*--str = c < 10 ? c + '0' : c + 'A' - 10;
//		} else {
//			*--str = '0';
//		}
//		if (frac && (digits == (decimals))) {
//			*--str = '.';
//		}
//	}
//	return serialPuts(portHandle, str);
//}
//
//// Allows us to fill the first 8 CGRAM locations with custom characters
//void SerialLCD::defineChar_P(uint8_t location, const uint8_t charmap[])
//{
//	location &= 0x7; // we only have 8 locations 0-7
//	print("?D");
//	print(location, DEC);
//	for (int i=0; i<8; i++) {
//		puti(pgm_read_byte(&charmap[i]), 2, 16);
//	}
//	delay(100); // pause to allow LCD EEPROM to program
//}
