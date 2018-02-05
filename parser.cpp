/*
 * parser.cpp
 *
 *  Created on: 17.04.2010
 *      Author: dplasa
 */

#include "parser.h"
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "scheduler.h"
#include "rslx_util.h"

// arduino millis() function
extern "C" unsigned long millis();

#define PMODULE "parser"

parser::parser_command parser::commands[MAX_PARSE_CALLBACKS] = {{0,},};
FILE *parser::fin = NULL;
FILE *parser::fout = NULL;
char parser::buf[MAX_PARSE_WORD];

parser Parser;

void parser::setup(FILE *_fin, FILE *_fout)
{
	fin = _fin;
	fout = _fout;
}

char parser::get_char(uint8_t timeout)
{
	// read next character from [fin]
	int c = ::fgetc(fin);
	for (; (c<=0) && timeout; --timeout)
	{
		_delay_ms(1);
		c = ::fgetc(fin);
	}
	// filter / ignore whitespaces that indicate next command
	if (( (char)c == '\n') || ( (char)c == '\r') || ( (char)c == '\t') || ( (char)c == '\v')  || ( (char)c == ';'))
		return 0;
	return (char)c;
}

uint8_t parser::get_word(uint8_t timeout)
{
  // skip trailing spaces
  char c = ' ';
  while ( c == ' ' )
    c = Parser.get_char();

  uint8_t len = 0;
  while (len < sizeof(buf))
  {
    if (c <= ' ')
      break;

    buf[len++] = c;
    c = Parser.get_char();
  }

  // terminating '\0'
  buf[len] = 0;
  return len;
}

// read from [in] parse next command and execute callbacks
void parser::update()
{
	char c = get_char(0);
	if (c <= 0)
		return;
	else if (c == 'h' ||  c == '?')
	{
		// list of all commands
		fprintf_P(fout, PSTR(PMODULE ": known commands:\n  ! - Reset\n  ?,h - Help\n"));
		parser_command * pc = commands;
		for (uint8_t i=0; i < MAX_PARSE_CALLBACKS; ++i, ++pc)
		{
			if (pc->cmd == '\0')
				continue;
			fprintf_P(fout, PSTR("  %c - %S\n"), pc->cmd, pc->name);
		}
	}
	else if (c == '!')
	{
		// perform a reset
		asm("push __zero_reg__");
		asm("push __zero_reg__");
		asm("ret");
	}
	else
	{
		bool unknown_command = true;
		// search for command [c]
		parser_command * pc = commands;
		for (uint8_t i=0; i < MAX_PARSE_CALLBACKS; ++i, ++pc)
		{
			if (pc->cmd == c)
			{
				pc->cb(pc->name, pc->arg);
				unknown_command = false;
				break;
			}
		}

		if (unknown_command)
		{
			fprintf_P(fout, PSTR(PMODULE ": unknown command '%c [%d]'\n"), c, c);
		}
	}
}

bool parser::registerCallback(char cmd, const char* name, parser_callback cb, void* arg)
{
	parser_command * pc = commands;
	for (uint8_t i=0; i < MAX_PARSE_CALLBACKS; ++i, ++pc)	
	{
		if (pc->cmd == '\0')
		{
			pc->cb = cb;
			pc->name = name;
			pc->cmd = cmd;
			pc->arg = arg;
			return true;
		}
	}
	return false;
}
