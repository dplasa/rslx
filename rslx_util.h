#ifndef _RSLX_UTIL_H
#define _RSLX_UTIL_H

#include <avr/pgmspace.h>
#include <stdio.h>

/* "tokenize" a string in PGM mem with delimiters in PGM mem
 * Returns: 
 *		size of current token in bytes
 * 		argument s will point to the start of the next token after return
 *		if no next token exists, s will point to end of string and 0 will be returned
 */
int8_t strtok_PP( const char*& s, const char*  delim );

/* search for character needle in PGM mem string haystack
 * Returns: true if found, false else
 */
//bool charfind_P(PGM_P haystack, char needle);

#define IS_DELIM(c) ((c == 0) || ( (char)c == ',') || ( (char)c == ' ') || ( (char)c == '=') || ( (char)c == '\t') || ( (char)c == '\n')  || ( (char)c == '\r')|| ( (char)c == '\v'))

//void rslx_fputs_P(FILE* f, const char* s);

#endif /* _RSLX_UTIL_H */
