/*--[manifest.h]--------------------------------------------------------------
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
typedef struct mapping_state {
    int     magic;
    manifest_type * p;        
    int     group, index;
    int     max_index;
    name_mapping * map;
} mapping_state;

void mapping_init(mapping_state * m, manifest_type * pManifest);
name_mapping * mapping_next(mapping_state * m);

void display_manifest(manifest_type * pManifest);
int duplicate_manifest(manifest_type * pDest, manifest_type * pSource);
int make_relative_manifest(U8 * , manifest_type * , manifest_type * );

