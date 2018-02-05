/*
 * ehandler.cpp
 *
 *  Created on: 13.04.2010
 *      Author: dplasa
 */

#include "ehandler.h"
#include "rslx_util.h"

extern error_entry* error_mem;

void do_errDump(FILE* stream, uint8_t maxErrorCount, const error_entry* ee, const char* error_names)
{
	fprintf_P(stream, PSTR("A T CNT Name\n"));
	for (uint8_t i=0; i<maxErrorCount; ++i, ++ee)
	{
		if ((*(uint8_t*) ee) != 0)
		{
			const error_entry& e = *ee;
			char etype = pgm_read_byte_near(&(errTypes[e.etype]));
			// lookup error name
			const char *ntoken = error_names;
			const char *s;
			uint8_t errn= 0;
			int len;
			do
			{
				s = ntoken;
				len = strtok_PP( ntoken, PSTR(" ,=\t\v\n\r") );
			} while (len && (errn++ < i));
			// s now points to beginning of state name or at the end of stnames
			if (len>8) len=8;
			char buf[9]; 
			strncpy_P(buf, s, len); buf[len]=0;
			fprintf_P(stream, PSTR("%c %c %3d %s\n"), (e.active ? '*' : ' '), etype, e.count, buf);
		}
	}
}

bool is_active(uint8_t maxErrorCount, const error_entry* ee)
{
	for (; maxErrorCount >0; --maxErrorCount, ++ee)
	{
		if (ee->active) return true;
	}
	return false;
}

bool is_any(uint8_t maxErrorCount, const error_entry* ee)
{
	for (; maxErrorCount>0; --maxErrorCount, ++ee)
	{
		if (*((uint8_t*) ee)) return true;
	}
	return false;
}
