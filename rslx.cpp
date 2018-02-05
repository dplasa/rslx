#include "rslx.h"
#include <stdio.h>

Stream* usestream = NULL;
unsigned long (*ticker)(void) = &millis;

int rslx_myput(char c)
{
	char res = usestream->print(c);
	if ('\n' == c) usestream->flush();
	return res;
}

int rslx_myget()
{
	return usestream->read();
}

void RSLX::setTickSource(unsigned long (*newTicker)(void))
{
  ticker = newTicker;
}

void RSLX::setup(Stream& astream, uint8_t eelayout)
{
  usestream = &astream;
  if (usestream)
    fdevopen((int(*)(char, FILE *)) &rslx_myput, (int(*)(FILE *)) &rslx_myget);
  Parser.setup(stdin, stdout);
  coop_scheduler::setup();
  var_registrar::eelayout = eelayout;
  var_registrar::loadDefaults();
  coop_scheduler::start();
}

void RSLX::update()
{
	// scheduler aufrufen
	coop_scheduler::update();
	
	// parser aufrufen
	Parser.update();
}

RSLX rslx;
