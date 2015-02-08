/*! \file rprintf.c \brief printf routine and associated routines. */
//*****************************************************************************
//
// File Name	: 'rprintf.c'
// Title		: printf routine and associated routines
// Author		: Pascal Stang - Copyright (C) 2000-2002
// Created		: 2000.12.26
// Revised		: 2003.5.1
// Version		: 1.0
// Target MCU	: Atmel AVR series and other targets
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

//#include <avr/pgmspace.h>
//#include <string-avr.h>
//#include <stdlib.h>
#include <stdarg.h>
//#include "global.h"
#include "rprintf.h"


#ifndef TRUE
	#define TRUE	-1
	#define FALSE	0
#endif

#define INF     32766	// maximum field size to print
//#define READMEMBYTE(a,char_ptr)	((a)?(pgm_read_byte(char_ptr)):(*char_ptr))

#ifdef RPRINTF_COMPLEX
	static unsigned char buf[128];
#endif

// use this to store hex conversion in RAM
//static char HexChars[] = "0123456789ABCDEF";
// use this to store hex conversion in program memory
//static prog_char HexChars[] = "0123456789ABCDEF";
static char code HexChars[] = "0123456789ABCDEF";

// function pointer to single character output routine
//static void (*rputchar)(unsigned char c);

// *** rprintf initialization ***
// you must call this function once and supply the character output
// routine before using other functions in this library
//void rprintfInit(void (*putchar_func)(unsigned char c))
//{
//	rputchar = putchar_func;
//}

// *** rprintfChar ***
// send a character/byte to the current output device
/*void rprintfChar(unsigned char c)
{
	// send character
	putchar(c);
}
*/
// *** rprintfStr ***
// prints a null-terminated string stored in RAM
void rprintfStr(char str[])
{
	// send a string stored in RAM
	// check to make sure we have a good pointer
	if (!str) return;

	// print the string until a null-terminator
	while (*str)
		putchar(*(str++));
}

// *** rprintfStrLen ***
// prints a section of a string stored in RAM
// begins printing at position indicated by <start>
// prints number of characters indicated by <len>
void rprintfStrLen(char str[], unsigned int start, unsigned int len)
{
	register int i=0;

	// check to make sure we have a good pointer
	if (!str) return;
	// spin through characters up to requested start
	// keep going as long as there's no null
	while((i++<start) && (*str++));
//	for(i=0; i<start; i++)
//	{
//		// keep steping through string as long as there's no null
//		if(*str) str++;
//	}

	// then print exactly len characters
	for(i=0; i<len; i++)
	{
		// print data out of the string as long as we haven't reached a null yet
		// at the null, start printing spaces
		if(*str)
			putchar( *str++);
		else
			putchar(' ');
	}

}

// *** rprintfProgStr ***
// prints a null-terminated string stored in program ROM
void rprintfProgStr(unsigned char *str)
{
	// print a string stored in program memory
	unsigned char c;

	// check to make sure we have a good pointer
	if (!str) return;
	
	// print the string until the null-terminator
	while(c = *str)
	{	str++;
		putchar((unsigned char)c);
	}
}

// *** rprintfCRLF ***
// prints carriage return and line feed
void rprintfCRLF(void)
{
	// print CR/LF
	putchar('\r');
	putchar('\n');
}

// *** rprintfu04 ***
// prints an unsigned 4-bit number in hex (1 digit)
void rprintfu04(unsigned char dat)
{
	// print 4-bit hex value
//	char Character = data&0x0f;
//	if (Character>9)
//		Character+='A'-10;
//	else
//		Character+='0';
	putchar( HexChars[dat&0x0f] );
}

// *** rprintfu08 ***
// prints an unsigned 8-bit number in hex (2 digits)
void rprintfu08(unsigned char dat)
{
	// print 8-bit hex value
	rprintfu04(dat>>4);
	rprintfu04(dat);
}

// *** rprintfu16 ***
// prints an unsigned 16-bit number in hex (4 digits)
void rprintfu16(unsigned short dat)
{
	// print 16-bit hex value
	rprintfu08(dat>>8);
	rprintfu08(dat);
}

// *** rprintfu32 ***
// prints an unsigned 32-bit number in hex (8 digits)
void rprintfu32(unsigned long dat)
{
	// print 32-bit hex value
	rprintfu16(dat>>16);
	rprintfu16(dat);
}

// *** rprintfNum ***
// special printf for numbers only
// see formatting information below
//	Print the number "n" in the given "base"
//	using exactly "numDigits"
//	print +/- if signed flag "isSigned" is TRUE
//	use the character specified in "padchar" to pad extra characters
//
//	Examples:
//	uartPrintfNum(10, 6,  TRUE, ' ',   1234);  -->  " +1234"
//	uartPrintfNum(10, 6, FALSE, '0',   1234);  -->  "001234"
//	uartPrintfNum(16, 6, FALSE, '.', 0x5AA5);  -->  "..5AA5"
void rprintfNum(char base, char numDigits, char isSigned, char padchar, long n)
{
	// define a global HexChars or use line below
	//static char HexChars[16] = "0123456789ABCDEF";
	char *p, buf[32];
	unsigned long x;
	unsigned char count;

	// prepare negative number
	if( isSigned && (n < 0) )
	{
		x = -n;
	}
	else
	{
	 	x = n;
	}

	// setup little string buffer
	count = (numDigits-1)-(isSigned?1:0);
  	p = buf + sizeof (buf);
  	*--p = '\0';
	
	// force calculation of first digit
	// (to prevent zero from not printing at all!!!)
	*--p = HexChars[x%base]; x /= base;
	// calculate remaining digits
	while(count--)
	{
		if(x != 0)
		{
			// calculate next digit
			*--p = HexChars[x%base]; x /= base;
		}
		else
		{
			// no more digits left, pad out to desired length
			*--p = padchar;
		}
	}

	// apply signed notation if requested
	if( isSigned )
	{
		if(n < 0)
		{
   			*--p = '-';
		}
		else if(n > 0)
		{
	   		*--p = '+';
		}
		else
		{
	   		*--p = ' ';
		}
	}

	// print the string right-justified
	count = numDigits;
	while(count--)
	{
		putchar(*p++);
	}
}

unsigned char Isdigit(char c)
{
	if((c >= 0x30) && (c <= 0x39))
		return TRUE;
	else
		return FALSE;
}



