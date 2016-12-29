
/* 
 * This code is released into the public domain. 
 */

/*------------------------------------------------------------------------------
 |
 | Hexdump
 | In:  ptr	-> pointing to region to dump
 |	size 	-> number of bytes to dump
 |	
 | Result goes to stdout
 | 
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

#define numBytesPerLine	16
#define numSpaces	5
#define numBytesForHex numBytesPerLine*3
#define numBytesInString numBytesForHex+numSpaces+numBytesPerLine

void hexdump( byte * ptr, int size )
{
	char strbuffer[ numBytesInString+1 ];
	char * curStr;
	int	numBytes;
	int	idx;

	strbuffer[ numBytesInString ] = '\0';
	while ( size ) 
	{
		memset( strbuffer, ' ', numBytesInString );
		numBytes = (size>numBytesPerLine) ? numBytesPerLine : size;

		curStr = strbuffer;
		for ( idx = 0; idx < numBytes ; idx++ ) 
		{
			char	c1, c2;

			c2 = ( *(ptr + idx) & 0xF ) + '0';
			c1 = (( *(ptr + idx) & 0xF0) >> 4) + '0';
			if ( c1 > '9' ) c1 += ('A'-'9'-1);
			if ( c2 > '9' ) c2 += ('A'-'9'-1);
			*(curStr++) = c1;
			*(curStr++) = c2;	
			curStr++;
		}		
		curStr = strbuffer + numBytesForHex + numSpaces;
		for ( idx = 0; idx < numBytes ; idx++ ) 
		{
			if ( isprint( *(ptr+idx) ) ) 
			{
				*(curStr++) = *(ptr+idx );
			} else 
			{
				*(curStr++) = '.';		
			}
		}
		puts( strbuffer );

		size -= numBytes;	
		ptr += numBytes;
	}	
	
}
