/*
 * vars.cpp
 *
 *  Created on: 15.10.2012
 *      Author: dplasa
 */

#include "vars.h"
#include "parser.h"
#include <stdlib.h>
#include "debug.h"
#include "rslx_util.h"

#include <math.h>
#include <stdlib.h>

var_registrar vars;
uint8_t var_registrar::vcount = 0;
uint8_t var_registrar::eelayout = 0;

uint8_t __ee_eelayout EEMEM = 0;

char var_base::buf[VAR_MAX_NAMELEN+1];

//
// special template instanciations
//

// INT Container
template<>
void varcontainer<int>::defaultToStr()
{
  buf[0] = '\0';
  if (_persistent)
    snprintf_P(buf, sizeof(buf), PSTR("%d"), eeprom_read_word((const uint16_t*)_default));
}

template<>
void varcontainer<long int>::defaultToStr()
{
  buf[0] = '\0';
  if (_persistent)
    snprintf_P(buf, sizeof(buf), PSTR("%ld"), eeprom_read_dword((const uint32_t*)_default));
}

// FLOAT Container
template<>
void varcontainer<float>::setValue(const char* val)
{
  *((float*)_value) = atof(val);
}

template<>
void varcontainer<float>::valueToStr()
{
  printfloat(*((float*)_value));
}

template<>
void varcontainer<float>::defaultToStr()
{
  buf[0] = '\0';
  if (_persistent)
    printfloat(eeprom_read_float((const float*)_default));
}


//
// VAR MANAGER
//
var_base* var_registrar::v_list[VAR_MAX_COUNT] = {0,};
var_registrar::var_registrar()
{
  // register the var_registrar's subparser in the global Parser
  // act on command 'v'
  Parser.registerCallback('v', PSTR("Var/Param"), var_registrar::parser, this);
}

uint8_t var_registrar::count()
{
  return vcount;
}

void var_registrar::add(var_base *sbase)
{
  if (vcount < VAR_MAX_COUNT)
    v_list[vcount++] = sbase;
}

void var_registrar::saveDefaults()
{
  var_base ** v = v_list;
  for (uint8_t i = 0; i < vcount; ++i, ++v)
  {
    (*v)->saveDefault();
  }
}

void var_registrar::parser(const char *name, void * arg __attribute__((unused)))
{
  char c = Parser.get_char();
  if (c == 'l' || c <= 0)
  {
    // dump all vars
    fprintf_P(Parser.fout, PSTR(" #|Name      |Value       |Default [v=%d]\n"), eeprom_read_byte(&__ee_eelayout));
    fprintf_P(Parser.fout, PSTR("--+----------+------------+-------------\n"));
    var_base ** v = v_list;
    for (uint8_t i = 0; i < vcount; ++i, ++v)
    {
      fprintf_P(Parser.fout, PSTR("%2d|%-10S|%c") , i, (*v)->_name, (*v)->modified() ? '~' : ' ');
      (*v)->valueToStr();
      fprintf_P(Parser.fout, PSTR("% 11s|"), var_base::buf);
      (*v)->defaultToStr();
      fprintf_P(Parser.fout, PSTR("% 11s\n"), var_base::buf);
    }
  }
  else if ((c=='d') || (c=='s'))
  {
    // load or save default
    var_base ** v = v_list;
    const bool save = (c=='s');
    for (uint8_t i = 0; i < vcount; ++i, ++v)
    {
      if(save)
        (*v)->saveDefault();
      (*v)->loadDefault();
    }
    fprintf_P(Parser.fout, PSTR("%S: all defaults %S\n"), name, save ? PSTR("saved") : PSTR("loaded") );
  }
  else if (c=='w')
  {
    do {
      // wert aendern: VW INDEX|NAME WERT

      // get var name
      uint8_t len = Parser.get_word();
      if (len == 0) break;

      //fprintf_P(Parser.fout, PSTR("name is >%s<, len=%d\n"), Parser.buf, len);

      // test if buf contains an index or name
      char *endptr;
      uint8_t varindex = strtol(Parser.buf, &endptr, 0);

      // search var name / index
      var_base ** v = v_list;
      if (*endptr != '\0')
      {
        //fprintf_P(Parser.fout, PSTR("search name == >%s<\n"), Parser.buf);
        for (varindex = 0; varindex < vcount; ++varindex, ++v)
        {
          if (0 == strcmp_P(Parser.buf, (*v)->_name))
            break;
        }
      }
      else
      {
        v += varindex;
        //fprintf_P(Parser.fout, PSTR("treat == >%s< as index %d\n"), Parser.buf, varindex);
      }
      if (varindex >= vcount)
      {
        fprintf_P(Parser.fout, PSTR("***Var %s not found\n") , Parser.buf);
        break;
      }

      // get var value
      len = Parser.get_word();
      //fprintf_P(Parser.fout, PSTR("value is >%s<, len=%d\n"), Parser.buf, len);

      // no line as expected
      if (len == 0) break;

      //fprintf_P(stdout, PSTR( "***Var %S found at %i\n") , v_list[varindex]->_name, varindex);
      (*v)->setValue(Parser.buf);
      (*v)->valueToStr();
      fprintf_P(Parser.fout, PSTR( "%S = %s\n" ), (*v)->_name, var_base::buf);
    } while (0);
  }
  else
  {
    fprintf_P(Parser.fout, PSTR("%S: unknown command '%c'\n"), name, c);
  }
}

void var_registrar::loadDefaults()
{
  // check if supposed structure of eemem is consinstent with
  // real structure of eemem

  bool ee_reinit = eeprom_read_byte(&__ee_eelayout) != eelayout;

  var_base ** v = v_list;
  for (uint8_t i = 0; i < vcount; ++i, ++v)
  {
    if (ee_reinit && ((*v)->persistent()))
      (*v)->saveDefault();
    (*v)->loadDefault();
  }
  eeprom_update_byte(&__ee_eelayout, eelayout);
}

// helper function to print out a float with var_registrar::fWidth digits
void var_base::printfloat(float number)
{
  snprintf_P(var_base::buf, sizeof(var_base::buf), PSTR("% .3f"), number);
//
//  // Extract the integer part of the number and print it to buffer
//  int32_t int_part = int32_t(truncf(number));
//  float remainder = fabs(number - int_part);
//
//  // print and get length of integer part
//  int8_t ilen = snprintf_P(var_base::buf, sizeof(var_base::buf), PSTR("%ld"), int_part);
//
//  // shifted reminder according to width
//  for (int8_t s = var_registrar::fWidth - ilen; s>0; --s)
//  {
//    remainder *= 10;
//  }
//  int32_t frac_part = int32_t(truncf(remainder));
//  int8_t flen = snprintf_P(var_base::buf, sizeof(var_base::buf), PSTR("%ld"), frac_part);
//
//  // fill buffer with '0'
//  memset(var_base::buf, 'x', sizeof(var_base::buf));
//
//  // shift integer part according to + or - number
//  char* b = var_base::buf;
//  int8_t len = sizeof(var_base::buf);
//  if (int_part >= 0)
//  {
//    *b++ = ' ';
//    --len;
//  }
//  ilen = snprintf_P(b, len, PSTR("%ld"), int_part);
//  var_base::buf[ilen]='.';
//  snprintf_P(b+ilen+2, flen, PSTR("%ld"), frac_part);
}
