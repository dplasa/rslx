#include "debug.h"

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
void hexdump(const void* _data, uint16_t len)
{
  uint8_t *data = (uint8_t*)_data;
  uint16_t i=0;
  do
  {
    // at the beginning of each 16 byte range print addresses
    if (i % 16 == 0) 
    {
      uint16_t ubound = MIN(len, i+16);
      fprintf_P(stdout, PSTR("%04x..%04x: "), i, ubound);
    }
    // write current byte
    fprintf_P(stdout, PSTR("%02x "), *data++);

    // increase i
    ++i;

    if (i % 16 && i < len)
      continue;

    // need a line wrap or data end
    // calc next 16er boundary
    uint16_t ubound = (16 - i%16)%16;
    for (int j = 0; j < ubound; ++j) fprintf_P(stdout, PSTR("   "));

    // print any visual text characters
    // calc number of bytes to print
    int16_t c = 16-ubound;
    // rewind data
    data -= c;
    for (; c >0; --c)
    {
       char tmp = *data++;
       if ((tmp < 32) || (tmp > 127)) tmp = '.';
       fprintf_P(stdout, PSTR("%c"), tmp);
    }
    fprintf_P(stdout, PSTR("\n"));
  } while (i < len);
}

