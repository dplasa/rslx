/*
 * scheduler.h
 *
 *  Created on: 10.10.2012
 *      Author: plasa
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define AVR_SCHEDULER_TASKS 8

#define makePeriodic(func, ms) do { Scheduler.registerTask(func, ms); } while(0)

typedef void ( * scheduler_task) ( void );

class coop_scheduler
{
public:

  coop_scheduler() {}
  ~coop_scheduler();

  // setup
  static void setup();

  // enable or disable scheduler
  // if task!= NULL given: enable or disable registered task
  static void stop(scheduler_task tsk = NULL);
  static void start(scheduler_task tsk = NULL);

  // is scheduler / task running?
  static uint8_t getStatus(scheduler_task tsk = NULL);

  // get scheduler Resolution (in kHz)
  static uint16_t getResolution();

  // register a function to be called as periodic task every interval ticks
  static bool registerTask(scheduler_task tsk, uint32_t interval);
  // remove a registered function - this can never fail
  static void removeTask(scheduler_task tsk);

  // must be called periodically to ensure scheduler's work
  static void update();

private:
  enum tsk_flags
  {
    tInactive = 0,
    tActive = 1,
    tRunning = 2,
    tStopped = 4
  };
  struct tsk_entry
  {
    uint8_t flags;      // flags
    scheduler_task tsk;	// function pointer
    uint32_t interval;	// intervall in Ticks
    uint32_t ticks;		  // next run @ ticks
    uint32_t rmin;		  // min execution time (us)
    uint32_t rmax;		  // max execution time (us)
  };
  static tsk_entry TL[AVR_SCHEDULER_TASKS];
  static uint8_t flags;
  static uint32_t ucount;
  static uint32_t smikros;
  static uint32_t rmax;
  static uint16_t freq;
  static void pasercb(const char* name, void *);
};

#endif /* SCHEDULER_H_ */
