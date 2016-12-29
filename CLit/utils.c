/*--[utils.c]----------------------------------------------------------------
 | Copyright (C) 2004 Digital Rights Software
 |
 | This file is part of "clit" (Convert LIT). 
 |
 | "clit" is free software; you can redistribute it and/or modify
 | it under the terms of the GNU General Public License as published by
 | the Free Software Foundation; either version 2 of the License, or
 | (at your option) any later version.
 |
 | This program is distributed in the hope that it will be useful,
 | but WITHOUT ANY WARRANTY; without even the implied warranty of
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 | GNU General Public License for more details.
 |
 | You should have received a copy of the GNU General Public License
 | along with this program; if not, write to the Free Software
 | Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 | 
 | The GNU General Public License may also be available at the following
 | URL: http://www.gnu.org/licenses/gpl.html
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext/litlib.h"
#include "clit.h"
#include "utils.h"

char * strmerge(char * head, char * body, char * tail)
{
    int len;
    char * str;

    len = 0;
    if (head) len += strlen(head);
    if (body) len += strlen(body);
    if (tail) len += strlen(tail); 
    len++;

    str = malloc(len);
    if (!str) {
        lit_error(0,"Unable to malloc %d bytes for a string.",len);
        return NULL;
    }
    str[0] = '\0';
    if (head) strcat(str,head);
    if (body) strcat(str,body);
    if (tail) strcat(str,tail);
    return str;
}



/*--[read_utf8_char]-----------------------------------------------------------
 |
 | This reads a single UTF8 character from a data stream, returning the
 | number of bytes consumed (element size) and filling in the Integer
 | value.
*/
int  read_utf8_char(U8 * pdata, int nBytes, U32 * pvalue)
{
    U32  c;
    unsigned char mask;
    int elsize, i;

    if (pvalue) *pvalue = -1;

    if (nBytes < 1) return -1;
    c = *(pdata);
    mask = 0x80;
    if (c & mask) {
        elsize = 0;
        while (c & mask) { mask >>= 1; elsize++;}
        if ((mask <= 1) || (mask == 0x40))  return -1;
    } else {
        elsize = 1;
    }

    if (elsize > 1) {
        if ((elsize) > nBytes) { return -1; }
        c &= (mask - 1);
        for (i = 1; i < elsize; i++) {
            if ( (*(pdata + i) & 0xC0) != 0x80)  return -1;
            c = (c << 6) | ( *(pdata + i) & 0x3F );
        }
    }
    if (pvalue) *pvalue = c;
    return elsize;
}

/*----------------------------------------------------------------------------
 | utf8_strlen(UTF8 * string)
 +--------------------------------------------------------------------------*/
int utf8_strlen(U8 * p)
{
    int units, bytes, len;
    U32 c;
    
    bytes = strlen(p)+1;
    units = 0;
    while (1) {
        len = read_utf8_char(p, bytes, &c);
        if ((!c) || (len <= 0)) break;
        units++;
        p += len;
    } 
    return units;
}

/*----------------------------------------------------------------------------
 | utf8_strmatch(UTF8 * s1, UTF8 * s2)
 +--------------------------------------------------------------------------*/
int utf8_strmatch(U8 * s1, U8 * s2)
{   
    int units, bytes1, bytes2, len;
    U32 c1, c2;

    bytes1 = strlen(s1)+1;
    bytes2 = strlen(s2)+1;
    units = 0;
    while (1) {
        len = read_utf8_char(s1, bytes1, &c1);
        if ((!c1) || (len <= 0)) break;
        s1 += len;
        bytes1 -= len;

        len = read_utf8_char(s2, bytes2, &c2);
        if ((!c2) || (len <= 0)) break;
        s2 += len;
        bytes2 -= len;

        if (c1 == c2) units++;
        else break;
    }  
    return units;
}

/*--[utf8_start]--------------------------------------------------------------
 | This starts the iteration of an UTF8 string.  
 */
void utf8_start(utf8_iter * iter, U8 * string) 
{
    memset(iter, 0, sizeof(utf8_iter));
    iter->ptr = string;
    iter->bytes = strlen(string);
}

/*--[utf8_peek]--------------------------------------------------------------
 | Reads the next UNICODE code point from an UTF8 string. 
 | Doesn't update the pointer
 */
U32 utf8_peek(utf8_iter * iter)
{
    int len;
    U32 c;

    len = read_utf8_char(iter->ptr,iter->bytes, &c);
    if (len <= 0) {
        c = 0;
    }
    return c;

}
/*--[utf8_next]--------------------------------------------------------------
 | Reads the next UNICODE code point from an UTF8 string. 
 | Updates the pointer to the next value
 */
U32 utf8_next(utf8_iter * iter) 
{
    U32 c;

    iter->last_ptr = iter->ptr; 
    iter->len = read_utf8_char(iter->ptr,iter->bytes, &c);
    if (iter->len <= 0) {
        c = 0;
        iter->last_ptr = NULL;
        iter->len = 0;
        iter->bytes = 0;
    }
    if (c != 0)  {
        iter->bytes -= iter->len;
        iter->ptr += iter->len;
    }
    return c;
}

/* --[utf8_store]--------------------------------------------------------------
 | Stores the previously read character into the output stream and returns
 | a length.
 */
int utf8_store(U8 * dest, utf8_iter * iter) {
    if (!iter->last_ptr) {
        *dest = 0;
        return 1;
    }
    memcpy(dest, iter->last_ptr, iter->len);
    return iter->len;
}

U8 * utf8_ptr(utf8_iter * iter)
{
    return iter->ptr;
}

/*--[utf8_next_token]----------------------------------------------------------
 | Reads a token (seperated by a single seperator) from the UTF8 iterator.
 | Returns the length (in bytes!) and the pointer to the beginning.
 | The length DOESN'T include the seperator, and the iterator SKIPS
 | the seperator.
 */
U8 * utf8_next_token(utf8_iter * iter, U32 seperator, int * pbytes)
{
    U8  * start;
    int bytes;
    U32 c;

    start = utf8_ptr(iter);
    bytes = 0;
    do {
        c = utf8_next(iter);
        if (c == seperator) break;
        bytes += iter->len;
    } while (c);
    if (pbytes) *pbytes = bytes;
    return start;
}
