#include "rslx_util.h"
#include <stdio.h>

/*bool charfind_P(PGM_P haystack, char needle)
{
	do
	{
		char c = pgm_read_byte_near(haystack++);
		if ( c == 0) return false;
		if (c == needle) return true;
	} while(1);
}*/

/*void rslx_fputs_P(FILE *f, const char* s)
{
	for (;;)
	{
		char c = pgm_read_byte_near(s++);
		if (c==0) return;
		::fputc(c, f);
	}
}
*/

int8_t strtok_PP( const char *& s, const char * delim )
{
	const  char * start = s;
	const char * cs = s;
	char c;
	int8_t len=0;
	bool skip = false;
  start:
	do	// skip leading delimiters (skip = false)
		// skip until next deilimiter (skip = true)
	{
		++len;
		c = pgm_read_byte_near(cs++);	// read next byte from s
		if (c == 0) 					// end of s?
			goto ende;
	} while ((IS_DELIM(c)) ^ skip);
	if (skip ^= true) goto start;
	len = (cs-start);
	while ((c!= 0) && (IS_DELIM(c)))	// skip trailing delimiters
	{
		// read next byte from s
		c = pgm_read_byte_near(cs++);
	} 
  ende:
	s = --cs;
	return --len;
}
