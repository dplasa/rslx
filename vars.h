/*
 * vars.h
 *
 *  Created on: 15.10.2012
 *      Author: Daniel Plasa
 *  Improvements 2013 by Tom Wank (better hash check)
 *  More improvements 2015 (DP) by including var position into hash
 */

#ifndef VARS_H_
#define VARS_H_

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define VAR_MAX_NAMELEN 11  // must be at least 11 chars to represent "−2147483648" (-2^31)
#define VAR_MAX_COUNT 16    // manage up to 16 variables

// the global vars registrar class
// can manage up to VAR_MAX_COUNT vars
struct var_base;
struct var_registrar
{
	/**
	 * if eelayout != saved eelayout, reinitialize eeprom
	 */
	static uint8_t eelayout;

	var_registrar();
	static void add(var_base *sbase);

	/**
	@parser Befehle:
	v variablen
	vw [name] [value] setze neue werte
	vw [index] [value]  setze neue werte
	vs save defaults (nur Param)
	vd load defaults
	*/
	static void parser(const char*name, void * sr);
	/**
	 * dump all variables to <to>
	 */
	static void dump(FILE* to);
	/**
	 * load all values from Flash/EEMEM repectively
	 */
	static void loadDefaults();
	/**
	 * Save all current values as default values (and to eeprom)
	 */
	static void saveDefaults();
  /**
   * Return the number of managed vars
   */
	static uint8_t count();
private:
	static uint8_t vcount;
	static var_base* v_list[VAR_MAX_COUNT];
};
extern var_registrar vars;



// Abstract base for either a variable or parameter or arbitrary length
struct var_base
{
  var_base(const char* name, void *val, void *def, uint8_t len) :
    _name(name), _value(val), _default(def), _len(len), _persistent(NULL != def)
  {
    var_registrar::add(this);
  }
  /**
   * @return value is different to default?
   */
  bool modified() const
  {
    if (_persistent)
    {
      // ASSERT: _len < sizeof(buf)
      eeprom_read_block(&buf, _default, _len);
      return memcmp(&buf, _value, _len);
    }
    return false;
  }
  /**
   * @return wether defaults are saved in eemem
   */
  bool persistent() const
  {
    return _persistent;
  }
  /**
   * load Default
   */
  void loadDefault()
  {
    if (_persistent)
      eeprom_read_block(var_base::_value, var_base::_default, _len);
  }
  /**
   * save Defaults (PARAM to EEMEM)
   */
  virtual void saveDefault()
  {
    if (_persistent)
      eeprom_update_block(_value, _default, _len);
  }
  /**
   * set VAR/PARAM value to val (cstring)
   */
  virtual void setValue(const char* val) = 0;
  /**
   * convert Value into buf
   */
  virtual void valueToStr() = 0;
  /**
   * convert Default value into buf
   */
  virtual void defaultToStr() = 0;

  const char* _name;
  void *_value;
  void *_default;
  uint8_t _len;
  bool _persistent;

  // helper
  static char buf[VAR_MAX_NAMELEN+1]; // with terminating \0
  static void printfloat(float);
};


template<typename TYPE>
struct varcontainer : public var_base
{
  varcontainer(const char* _name, TYPE *_val, TYPE *_def) :
    var_base(_name, _val, _def, sizeof(TYPE))
  { }
  void setValue(const char* val)
  {
    *((TYPE*)_value) = strtol(val, NULL, 0);
  }
  void valueToStr()
  {
    snprintf_P(buf, sizeof(buf), PSTR("%ld"), (long int) *((TYPE*)_value));
  }
  void defaultToStr();
};

#define VAR(X, TYPE, DEF) \
	TYPE X = DEF;	\
	const char name_##X[] PROGMEM = #X; \
	varcontainer<TYPE> vc_##X ( name_##X , &X, NULL );

#define PARAM(X, TYPE, DEF) \
	TYPE X = DEF;	\
	TYPE ee_##X EEMEM = DEF; \
	const char name_##X[] PROGMEM = #X; \
	varcontainer<TYPE> pc_##X ( name_##X , &X , &ee_##X );

#endif /* VARS_H_ */
