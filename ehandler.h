/*
 * ehandler.h
 *
 *  Created on: 13.04.2010
 *      Author: dplasa
 */

#ifndef EHANDLER_H_
#define EHANDLER_H_

#include "parser.h"
#include <avr/pgmspace.h>

enum ErrorTypes
{
	E_DEBUG,
	E_WARNING,
	E_ERROR,
	E_FATAL
};

static const char errTypes[] PROGMEM = "DWEF";

#define ERROR_COUNT_BITS 5
#define ERR_MAX_COUNT ((1<<ERROR_COUNT_BITS)-1)

struct error_entry
{
	uint8_t count	:ERROR_COUNT_BITS;
	uint8_t etype	:2;
	uint8_t active	:1;
};

// forward
void do_errDump(FILE* stream, uint8_t maxErrorCount, const error_entry* ee, const char* error_name);
bool is_any(uint8_t maxErrorCount, const error_entry* ee);
bool is_active(uint8_t maxErrorCount, const error_entry* ee);

#define ERRORS( ... ) \
	enum error_enum  { __VA_ARGS__ }; \
	static const uint8_t error_val [] PROGMEM = { __VA_ARGS__ }; \
	static const char error_names [] PROGMEM = #__VA_ARGS__; \
	error_entry error_mem  [ sizeof(error_val) / sizeof (uint8_t) ]; \
	struct error_memory \
	{ \
		void set(error_enum x, ErrorTypes et=E_ERROR) \
		{ \
			if (et > error_mem[x].etype) error_mem[x].etype = et; \
			if (! error_mem[x].active) \
			{ \
				if (++error_mem[x].count == 0) \
					error_mem[x].count = ERR_MAX_COUNT; \
			} \
			error_mem[x].active = 1; \
		} \
		void signal(error_enum x, ErrorTypes et=E_ERROR) \
		{ \
			if (et > error_mem[x].etype) error_mem[x].etype = et; \
			if (++error_mem[x].count == 0) \
				error_mem[x].count = ERR_MAX_COUNT; \
			error_mem[x].active = 0; \
		} \
		void clear(error_enum x) \
		{ \
			error_mem[x].active = 0; \
		} \
		bool test(error_enum x) \
		{ \
			return error_mem[x].active; \
		} \
		bool any() { return is_any(sizeof(error_val) / sizeof (uint8_t), (const error_entry*) error_mem); } \
		bool active() { return is_active(sizeof(error_val) / sizeof (uint8_t), (const error_entry*) error_mem); } \
		private: \
		void reset() \
		{	\
			for (uint8_t i=0; i < sizeof(error_val) / sizeof (uint8_t); ++i) \
			{ \
				((uint8_t&)error_mem[i]) = 0; \
			} \
		} \
		public: \
		static void parser(const char* name, void * em) \
		{ \
			char c = Parser.get_char(); \
			if (c == 'l' || c <= 0) \
			{ \
				if (is_any(sizeof(error_val) / sizeof (uint8_t), (const error_entry*) error_mem)) \
					do_errDump(stdout, sizeof(error_val) / sizeof (uint8_t), error_mem, error_names); \
				else \
					fprintf_P(Parser.fout, PSTR("%S: none\n"), name);\
			} \
			else if (c == 'r') \
			{ \
				for (uint8_t i=0; i < sizeof(error_val) / sizeof (uint8_t); ++i) \
				{ \
					((uint8_t&)error_mem[i]) = 0; \
				} \
				fprintf_P(Parser.fout, PSTR("%S: cleared\n"), name);\
			}	\
			else \
				fprintf_P(Parser.fout, PSTR("%S: unknown command '%c'\n"), name, c);\
		} \
		error_memory() \
		{ \
			Parser.registerCallback('e', PSTR("Errors"), parser, this); \
		} \
	}; \
	error_memory Errors;
	
#endif /* EHANDLER_H_ */
