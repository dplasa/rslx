#include "bits.h"
#include "parser.h"
#include <Arduino.h>


uint32_t BITS::bits_in = 0;
uint32_t BITS::bits_out = 0;
uint32_t BITS::bits_force  = 0;
uint32_t BITS::bits_mask = -1;

char BITS::buf[33] = {0,};

BITS Bits;

BITS::BITS()
{
	Parser.registerCallback('b', PSTR("Bits"), bits_parser, NULL);
}

void BITS::bits_parser(const char* name, void*)
{
	char c = Parser.get_char();
	if (c <= 0)
	{
		for (uint8_t i=0; i<8;++i) ::fputc(' ', Parser.fout);	// "        3         2         1"
		for (uint8_t i='3'; i>'0';--i)
		{
			::fputc(i, Parser.fout);
			for (uint8_t i=0; i<9;++i) ::fputc(' ', Parser.fout);
		} ::fputc('\n', Parser.fout);
		for (uint8_t i=0; i<7;++i) ::fputc(' ', Parser.fout);
		for (int8_t i=31; i>=0;--i)
		{
			::fputc('0' + (i%10), Parser.fout);
		}															// "      10987654321098765432109876543210"
		char* s = buf+sizeof(buf)-1; *s = 0;
		uint32_t mask = 1;
		for (uint8_t i = 0; i<32;++i)
		{
			char c = '-';
			if (bits_in & mask)
			{
				if (bits_out & mask)
					c = 'P';
				else
					c = 'I';
			} else if (bits_out & mask)
				c = 'O';
			*--s = c;
			mask <<= 1;
		}
		fprintf_P(Parser.fout, PSTR("\n  Dir: %s"), s);
		s = buf+sizeof(buf)-1; *s = 0;
		mask = 1;
		for (uint8_t i = 0; i<32;++i)
		{
			char c = (::digitalRead(i)) ? '1' : '0';
			*--s = c;
			mask <<= 1;
		}
		fprintf_P(Parser.fout, PSTR("\n Pins: %s"), s);
		s = buf+sizeof(buf)-1; *s = 0;
		mask = 1;
		for (uint8_t i = 0; i<32;++i)
		{
			char c = '0';
			if (bits_mask & mask)
			{
				c = '1';
			}
			*--s = c;
			mask <<= 1;
		}
		fprintf_P(Parser.fout, PSTR("\n Mask: %s"), s);
		s = buf+sizeof(buf)-1; *s = 0;
		mask = 1;
		for (uint8_t i = 0; i<32;++i)
		{
			char c = '0';
			if (bits_force & mask)
			{
				c = '1';
			}
			*--s = c;
			mask <<= 1;
		}
		fprintf_P(Parser.fout, PSTR("\nForce: %s"), s);
		s = buf+sizeof(buf)-1; *s = 0;
		for (uint8_t i = 0; i<32;++i)
		{
			char c = '0';
			if (digitalRead(i))
			{
				c = '1';
			}
			*--s = c;
		}
		fprintf_P(Parser.fout, PSTR("\nTotal: %s\n"), s);
	}
	else if ((c == 'f') || (c == 'm'))
	{
		uint32_t &target = (c=='f') ? bits_force : bits_mask;
		// force / mask bits setzen oder l�schen
		bool doremove = false;
		uint8_t bit = 0;
		do 
		{
			// skip leading blanks
			do 
			{
				c = Parser.get_char();
			} while	( c == ' ' );
			
			// no line as expected?
			if (c == '\n') break;
			
			// check for '+' or '-' prefix
			doremove = (c == '-');	// else c == '+' or Number --> set!
			// skip '+' / '-'
			if( c == '-'  || c == '+' ) c = Parser.get_char();

			// no line as expected?
			if (c == '\n') break;

			// get bit number
			while (c >= '0' && (c <= '9'))
			{
				bit *= 10;
				bit += (c - '0');
				c = Parser.get_char();
			}

			// do it!
			uint32_t mask = (1<<bit);
			if (doremove) 
				target &= ~mask;
			else
				target |= mask;
			break;
		} while (1);
	}
}

void BITS::pinMode(const uint8_t pin, const uint8_t mode)
{
	::pinMode(pin, mode);
	uint32_t mask = (1<<pin);
	if (mode == INPUT)
	{
		bits_in |= mask;
		bits_out &= ~mask;
	}
	else if (mode == OUTPUT)
	{
		bits_in &= ~mask;
		bits_out |= mask;
	}
	else if (mode == INPUT_PULLUP)
	{
		bits_in |= mask;
		bits_out |= mask;
	}
}

uint8_t BITS::digitalRead(const uint8_t pin)
{
	uint32_t res = (::digitalRead(pin) << pin);
	res	&= bits_mask;
	res	|= bits_force;
	return ((res >> pin) & 0x01);
}

void BITS::digitalWrite(const uint8_t pin, const uint8_t value)
{
	uint32_t res = value ? (1 << pin) : 0 ;
	res &= bits_mask;
	res |= bits_force;
	::digitalWrite(pin, ((res >> pin) & 0x01));
}
