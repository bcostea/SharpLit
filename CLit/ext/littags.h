/*--[littags.h]----------------------------------------------------------------
 | Copyright (C) 2002 Dan A. Jackson 
 |
 | This file is part of the "openclit" library for processing .LIT files.
 |
 | "Openclit" is free software; you can redistribute it and/or modify
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
#ifndef LITTAGS_H
#define LITTAGS_H

#include <stdlib.h>  /* need NULL */
#include "litlib.h"

typedef lit_attr_map attrmap;

extern attrmap tagmap0[];
extern const int tagcount;
extern attrmap * tagtoattr[109];
extern char * tagtoname[109];

extern attrmap meta_attr[];
extern const int meta_tagcount;
extern attrmap * meta_tagtoattr[43];
extern char * meta_tagtoname[43];

#endif
