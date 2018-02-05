/*
 * states.h
 *
 *  Created on: 23.02.2010
 *      Author: dplasa
 */

#ifndef STATES_H_
#define STATES_H_

#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "rslx_util.h"

extern unsigned long (*ticker)(void);

// save current and up to 15 previous states (16=15+1)
// (use a power of 2 for best code size)
#define MAX_STATES_HISTORY 16

// max length of state name
#define MAX_STATE_NAME 10

// struct of a state history entry
// 32 bit milli tick counter
//  8 bit uint state
struct states_history_entry
{
	uint32_t ticks;
	uint8_t state;
};
// typedef
typedef states_history_entry states_history[MAX_STATES_HISTORY];

// some common stuff in base class
struct states_base
{
  uint8_t histp;	// pouints too the current state (in history)
  uint8_t histc;	// number of entries in states history
  states_history history;
  const char* name;
  const char* stnames;
  static char buf[MAX_STATE_NAME+1];

  explicit states_base(const char* _name, const char* _stnames) : histp(0), histc(1), name(_name), stnames(_stnames) 
  {
    // start with histc == 1 as there is always the inital state
  }

	void statename(uint8_t cstate)
	{
    // lookup state name
    const char *ntoken = stnames;
    const char *s;
    uint8_t i = 0;
    int8_t len;
    do
    {
      s = ntoken;
      len = strtok_PP( ntoken, PSTR(" ,=\t\v\n\r") );
    } while (len && (i++ < cstate));
    // cname now points to beginning of state name or at the end of stnames
    if (len>MAX_STATE_NAME) len=MAX_STATE_NAME;
    strncpy_P(buf, s, len); buf[len]=0;
	}

	void dump(FILE* stream)
	{
		uint32_t cticks = ticker();
		fprintf_P(stream, PSTR("Trace %S:\nStart    [+Age ] <##> Name\n"), name);
                uint8_t current = histp;
		for (uint8_t i = histc ; i > 0; --i)
		{
			const states_history_entry& e = history[current];
			uint8_t cstate = e.state;
			uint32_t sticks = e.ticks;
			statename(cstate);
		   	fprintf_P(stream, PSTR("%08lu [%5ld] <%02d> %s\n"), sticks, (cticks-sticks), cstate, buf);
			cticks = sticks;
			--current %= MAX_STATES_HISTORY;
		}
	}
};


// the global state machine registrar class
// can manage up to MAX_STATE_MACHINES machines
#define MAX_STATE_MACHINES	8

struct state_registrar
{
	state_registrar();
	static void add_sm(states_base *sbase);
	static void parser(const char* name, void * sr);
	static void dump(FILE* to);
private:
	static uint8_t scount;
	static states_base* sm_list[MAX_STATE_MACHINES];
	states_base* find_base(const char*);
};

extern state_registrar gState_Registrar;



#define STATES(X,...) \
	enum states_enum_##X  { __VA_ARGS__ }; \
	static const uint8_t states_val_##X [] PROGMEM = { __VA_ARGS__ }; \
	class states_##X : public states_base \
	{ \
	public: \
		states_##X() : states_base( PSTR(#X), PSTR(#__VA_ARGS__) ) \
		{ \
			history[0].ticks = 0;  /* set inital state (first state enum) */  \
			history[0].state = (state = (states_enum_##X ) pgm_read_byte_near(&(states_val_##X[0]))); \
			gState_Registrar.add_sm( this ); \
		} \
		states_##X & operator= ( states_enum_##X newstate ) \
		{ \
			++histp %= MAX_STATES_HISTORY; /* go to next history entry */ \
			if (++histc > MAX_STATES_HISTORY) histc = MAX_STATES_HISTORY; /* adjust history buffer count and save state */ \
			history[histp].state = (uint8_t)(state = newstate); \
			history[histp].ticks = ticker(); \
			return *this; \
		} \
		inline __attribute__((__always_inline__)) operator int() const \
		{ \
			return (int)state; \
		} \
		inline uint32_t age() const \
		{ \
			return ticker() - history[histp].ticks; \
		} \
		inline enum states_enum_##X  prev(uint8_t depth=1)  const \
		{ \
			return (states_enum_##X)history[(histp-depth+MAX_STATES_HISTORY)%MAX_STATES_HISTORY].state; \
		} \
	private: \
		states_enum_##X state; \
	}; \
	states_##X X;

#endif /* STATES_H_ */
