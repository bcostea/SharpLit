
/*--[clit.h]-------------------------------------------------------------------
 | Copyright (C) 2002, 2003 Dan A. Jackson
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
#ifndef CLIT_H
#define CLIT_H

extern int  printed_error;
extern char * readingFilename;
extern char * writingFilename;
void show_error(int what, char * fmt, ...);
void show_information(char * fmt, ...);


#endif
