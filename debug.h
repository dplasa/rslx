/*
 * debug.h
 *
 *  Created on: 10.10.2012
 *      Author: dplasa
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>
#include <avr/pgmspace.h>

#ifdef DEBUG_SILENCE
#define DEBUG(fmt, ...) do {} while (0)
#define hexdump(data, len) do {} while (0)
#else
#define DEBUG(fmt, ...)	do { \
		fprintf_P(stdout, PSTR(fmt "\n"), ##__VA_ARGS__); } while (0)

void hexdump(const void* _data, uint16_t len) __attribute__((weak));
#endif

#endif /* _DEBUG_H */
