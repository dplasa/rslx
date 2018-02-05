/*
 * scheduler.cpp
 *
 *  Created on: 03.02.2010
 *      Author: plasa
 */

#include <stddef.h>
#include "scheduler.h"
#include "debug.h"
#include "parser.h"
#include "rslx_util.h"

// time function
extern unsigned long (*ticker)(void);
extern "C" unsigned long micros();

coop_scheduler::tsk_entry coop_scheduler::TL[AVR_SCHEDULER_TASKS] = {{0,},};
uint8_t coop_scheduler::flags = 0;
uint32_t coop_scheduler::ucount = 0;
uint32_t coop_scheduler::smikros = 0;
uint16_t coop_scheduler::freq = 0;

void coop_scheduler::setup()
{
  Parser.registerCallback('S', PSTR("Scheduler"), coop_scheduler::pasercb, NULL);
}

void coop_scheduler::pasercb(const char* name, void *)
{
  char c = Parser.get_char();
  if (c <= 0)
  {
    fprintf_P(Parser.fout, PSTR("%S [sys: %ld:%ld]"), name, ticker(), micros());
    if (flags) fprintf_P(Parser.fout, PSTR(" active @ %i Hz\n"), freq);

    tsk_entry* te = TL;
    for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
    {
      if ((te->flags & tActive) )
      {
        fprintf_P(Parser.fout, PSTR(" Tsk %i @%ld ticks, %S, %ld ... %ld us\n"), i, te->interval, (te->flags & tStopped) ? PSTR("stopped") : PSTR("active"), te->rmin, te->rmax );
      }
    }
  }
  else if ( c == '0')
  {
    stop();
    fprintf_P(Parser.fout, PSTR("%S stopped\n"), name);
  }
  else if ( c == '1')
  {
    start();
    fprintf_P(Parser.fout, PSTR("%S started\n"), name);
  }
  else
  {
    fprintf_P(Parser.fout, PSTR("%S: unknown command '%c'\n"), name, c);
  }
}

void coop_scheduler::update()
{
  if (!flags) return;

  ++ucount;

  uint32_t uticks = micros();

  if ((uticks - smikros))
  {
    ucount *= 1000000; ucount /= (uticks - smikros);
    freq = ucount;
    ucount = 0;
    smikros = uticks;
  }

  // check which callback need to be run
  uint32_t ticks = ticker();
  tsk_entry* te = TL;

  for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
  {
    if (te->flags == tActive)
    {
      if (te->ticks <= ticks)
      {
        te->flags |= tRunning;	// set running flag
        // calc next ticks counter for execution
        te->ticks = ticks + te->interval;
        uticks = micros();
        te->tsk();			// execute user task function
        te->flags &= ~tRunning;	// clear running flag
        uticks = micros() - uticks;
        if (uticks < te->rmin) te->rmin = uticks;
        if (uticks > te->rmax) te->rmax = uticks;
      }
    }
  }
}

coop_scheduler::~coop_scheduler()
{
  stop();
}

void coop_scheduler::stop(scheduler_task tsk)
{
  // clear run flag
  if (NULL == tsk)
  {
    flags &= ~tActive;
    return;
  }

  tsk_entry* te = TL;
  for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
  {
    if (te->tsk == tsk)
    {
      te->flags |= tStopped;
      break;
    }
  }
}

void coop_scheduler::start(scheduler_task tsk)
{
  // set run flag
  if (NULL == tsk)
  {
    flags |= tActive;
    ucount = 0;
    smikros = micros();
    return;
  }

  tsk_entry* te = TL;
  for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
  {
    if (te->tsk == tsk)
    {
      te->flags &= ~tStopped;
    }
  }
}

uint8_t coop_scheduler::getStatus(scheduler_task tsk)
{
  if (NULL == tsk) return flags;

  tsk_entry* te = TL;
  for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
  {
    if (te->tsk == tsk)
    {
      return te->flags;
    }
  }
  return 0;
}

uint16_t coop_scheduler::getResolution()
{
  return (flags) ? freq : 0;
}

bool coop_scheduler::registerTask(scheduler_task tsk, uint32_t interval)
{
  if (tsk == NULL)
    return false;

  tsk_entry* te = TL;
  for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
  {
    if (te->flags == tInactive)
    {
      te->tsk = tsk;
      te->interval = interval;
      te->ticks = 0;  // run @ next possible update
      te->rmin = (1UL<<31);
      te->rmax = 0;
      te->flags = tActive; // mark as valid entry
      return true;
    }
  }
  return false;
}

void coop_scheduler::removeTask(scheduler_task tsk)
{
  tsk_entry* te = TL;
  for (uint8_t i= 0; i < AVR_SCHEDULER_TASKS; ++i, ++te)
  {
    if (te->tsk == tsk)
    {
      te->flags = tInactive;	// invalid
    }
  }
}
