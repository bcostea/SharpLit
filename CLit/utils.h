/*--[utils.h]----------------------------------------------------------------
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

typedef struct utf8_iter {
	U8 	* ptr; 		
	int	bytes;
	U8	* last_ptr;
	int	len;
} utf8_iter;

void utf8_start(utf8_iter *,U8 * string);
U32  utf8_next(utf8_iter *);
U8 * utf8_next_token(utf8_iter * iter, U32 seperator, int * pbytes);
U32  utf8_peek(utf8_iter *);
U8 * utf8_ptr(utf8_iter *);
int  utf8_store(U8 *, utf8_iter *);

char * strmerge(char * head, char * body, char * tail);
int  read_utf8_char(U8 * pdata, int nBytes, U32 * pvalue);
int utf8_strlen(U8 * p);
int utf8_strmatch(U8 * s1, U8 * s2);
