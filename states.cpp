/*
 * states.cpp
 *
 *  Created on: 17.06.2010
 *      Author: dplasa
 */

#include "states.h"
#include "parser.h"
#include "debug.h"
#include <stdlib.h>

uint8_t state_registrar::scount = 0;
states_base* state_registrar::sm_list[MAX_STATE_MACHINES];
char states_base::buf[MAX_STATE_NAME+1];

state_registrar::state_registrar()
{
	// register the state_registrar's subparser in the global Parser
	// act on command 'z'
	Parser.registerCallback('z', PSTR("States"), state_registrar::parser, this);
}

void state_registrar::add_sm(states_base *sbase)
{
	if (scount < MAX_STATE_MACHINES)
		sm_list[scount++] = sbase;
}

states_base* state_registrar::find_base(const char *nori)
{
  // test if buf contains an index or name
  char *endptr;
  uint8_t smindex = strtol(nori, &endptr, 0);
  // search var name / index
  states_base ** sb = sm_list;
  if (*endptr != '\0')
  {
    //fprintf_P(Parser.fout, PSTR("search name == >%s<\n"), nori);
    for (smindex = 0; smindex < scount; ++smindex, ++sb)
    {
      if (0 == strcmp_P(nori, (*sb)->name))
        return *sb;
    }
  }
  else
  {
    //fprintf_P(Parser.fout, PSTR("treat == >%s< as index %d\n"), nori, smindex);
    sb += smindex;
  }

  if (smindex >= scount)
  {
    fprintf_P(Parser.fout, PSTR("***SM %s not found\n") , nori);
    return NULL;
  }
  return *sb;
}

void state_registrar::parser(const char *name, void * arg)
{
  state_registrar& sr = *(state_registrar*)arg;
  // get command
  char c = Parser.get_char();
  if (c == 'l' || c <= 0)
  {
    // dump a list of all available state machines
    sr.dump(Parser.fout);
  }
  else if ('0' <= c && c < '0'+sr.scount)
  {
    // dump by index
    sm_list[c-'0']->dump(Parser.fout);
  }

  else if ('r' == c || 'w' == c)
  {
    // reset a state machine
    Parser.get_word();
    states_base* sb = sr.find_base(Parser.buf);
    if (sb)
    {
      (++(sb->histp)) %= MAX_STATES_HISTORY;
      uint8_t newstate = 0;
      if (c=='w')
      {
        Parser.get_word();
        char* endptr;
        newstate = strtol(Parser.buf, &endptr, 0);
        if (*endptr != '\0')
        {
          for (uint8_t s = 0; s < 32; ++s)
          {
            sb->statename(s);
            if (0==strcmp(sb->buf, Parser.buf))
            {
              newstate = s;
              break;
            }
          }
        }
      }
      sb->history[sb->histp].state = newstate;
      sb->history[sb->histp].ticks = ticker();
      sb->statename(newstate);
      fprintf_P(Parser.fout, PSTR("*** SM %S: jumping to %s\n"), sb->name, sb->buf);
    }
  }
  else
    fprintf_P(Parser.fout, PSTR("%S: unknown command '%c'\n"), name, c);
}

void state_registrar::dump(FILE* to)
{
  fprintf_P(to, PSTR(" #|Name      |State     |Age\n" \
                     "--+----------+----------+----------\n"));

	for (uint8_t i = 0; i < scount; ++i)
	{
	  states_base* sb = sm_list[i];
	  sb->statename(sb->history[sb->histp].state);
		fprintf_P(to, PSTR("%2d|%-10S|%-10s|%ld\n"), i, sm_list[i]->name, states_base::buf, ticker()- sb->history[sb->histp].ticks);
	}
}

// glocal instance of the Registrar
state_registrar gState_Registrar;
