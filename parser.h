/*
 * parser.h
 *
 *  Created on: 17.04.2010
 *      Author: dplasa
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <avr/pgmspace.h>

class parser;
typedef void ( * parser_callback) (const char*, void* );
#define MAX_PARSE_CALLBACKS 8
#define MAX_PARSE_WORD 16

class parser
{
public:
  // a global buffer
  static char buf[MAX_PARSE_WORD];

  // FILE to read from and write to
  static FILE *fin, *fout;

	// set user defineable stream for input and output
	static void setup(FILE *_fin=stdin, FILE *_fout=stdout);

	// read from [in] parse next command and execute callbacks
	void update();

  // get next char from [fin]
  static char get_char(uint8_t timeout = 10);

  // get next word from [fin], skipping trailing whitespaces
  static uint8_t get_word(uint8_t timeout = 10);

	// register a parser callback function to be called as parser recognizes command
	static bool registerCallback(char cmd, const char* name, parser_callback cb, void* arg);
	// remove a registered callback
	static bool removeCallback(char cmd);

private:
	struct parser_command
	{
		char cmd;
		const char* name;
		void *arg;
		parser_callback cb;
	};
	static parser_command commands[MAX_PARSE_CALLBACKS];
};

extern parser Parser;

#endif /* PARSER_H_ */
