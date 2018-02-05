/*
 * RS Legacy Extensions
 *
 * rslx.h
 *
 *  Created on: 14.10.2012
 *      Author: dplasa
 *
 * Revision History:
 * 1.17: fix handling of state history buffer
 * 1.16: provide custom time tick function
 *       rewrite vars handling
 *       more flexibility on state parser including state machine reset and state jumps
 * 1.15: fixed var/param "vw" cases that altered always var @ offset 0
 * 1.14: add hexdump
 * 1.13: make buffer in bits.h private
 * 1.12: rewrite eeprom re-initialization
 * 1.11: minor fixes
 * 1.10: include coop_scheduler into rslx class context
 * 1.9:  remove rslx_fputs_P, add bit masking/forcing (incomplete on OUTPUTs)
 *		   add const char* name to parser callbacks
 * 1.8:  fix states duration output, increase to history depth 16
 *		   add parser help module names
 *		   add some bit / pins IO handling
 * 1.7:  fix parser get_char timeout
 * 1.6:  var/params now use PGMSPACE strings
 * 1.5:  rewritten VAR handling, added PARAM handling
 * 1.4:  leaner (?) code with rslx_fputs_P
 * 1.3:  change rslx_setup to rslx::setup, change to Arduino class Stream reference as parameter
 * 1.2:  adds operator int() for state machines
 *       change parsing of strtok to strtok_PP
 * 1.1:  initial version for arduino
 * 1.0:  initial version for atmel / avr-gcc
 */

#ifndef _RSLX_H
#define _RSLX_H

#define RSLX_MAJOR 1
#define RSLX_MINOR 17
#define RSLX_VERSION ((RSLX_MAJOR<<16) | RSLX_MINOR)

#include "debug.h"
#include "states.h"
#include "scheduler.h"
#include "parser.h"
#include "vars.h"
#include "ehandler.h"
#include "bits.h"
#include "sma.h"

#include <stdio.h>
#include <Arduino.h>


struct RSLX : public coop_scheduler
{
  static void setup(Stream &astream = Serial, uint8_t eelayout = 0);
  static void update();
  static void setTickSource(unsigned long (*ticker)(void));
};
extern RSLX rslx;

#endif /* _RSLX_H */
