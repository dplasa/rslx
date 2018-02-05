/*
 * bits.h
 *
 *  Created on: 31.10.2012
 *      Author: dplasa
 */

#ifndef _BITS_H
#define _BITS_H

#include <stdint.h>

struct BITS
{
	BITS();
	static void pinMode(const uint8_t pin, const uint8_t mode);
	static uint8_t digitalRead(const uint8_t pin);
	static void digitalWrite(const uint8_t pin, const uint8_t value);
private:
	static void bits_parser(const char*, void*);
	static uint32_t bits_in;
	static uint32_t bits_out;
	static uint32_t bits_force;
	static uint32_t bits_mask;
	static char buf[33];
};

extern BITS Bits;

#endif /* _BITS_H */
