/*--[display.c]--------------------------------------------------------------
 | Copyright (C) 2004, Digital Rights Software
 |
 | This file is part of the "clit" (Convert LIT) program. 
 |
 | Convert LIT is free software; you can redistribute it and/or modify
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
#include "litlib.h"

extern void hexdump( unsigned char * ptr, int size );
/*
 | This file contains code to display information about a LIT file to
 | faciliate debugging.
 */

int display_lit(lit_file * lit, int public_only)
{
    entry_type  * entry;
    int size;
    unsigned char * p; 
    int real_size;
    int err;

    entry = lit->entry;
 
    while (entry)
    {
        printf("\"%s\" S: %llx Offset: %llx Size: %llx\n",
            &entry->name, entry->section, entry->offset, entry->size);
                
        /* Must skip the "/" entry, it has an absurd size */
        if ( ((public_only == 0) || (entry->section == 0)) &&
             ((entry->size) && (strcmp(&entry->name,"/") != 0)) )
        {
                err = lit_get_file(lit,&entry->name,&p,&real_size);
                if (err) { 
                     fprintf(stderr, "Error displaying %s: %d\n",
                         &entry->name, err);
                }
                else {
                        size = (unsigned int)(entry->size);

                        /* Very simplistic way to go UTF16->UTF8*/
                        if ( (real_size > 4) && 
                             (p[0] == 0xFF) && 
                             (p[1] == 0xFE) &&
                             (p[3] == 0) )
                        {
                             int i, j;

                             j = 0; 
                             for (i = 2; i < real_size; i+=2 )
                             {
                                p[j++] = p[i];
                             } 
                             size = j;
                        }
                        if (size > 8192) size = 8192;
                        if (p) {
                                hexdump(p, size);
                                free(p);
                        }
                }
        }
        entry = entry->next;
    }
    return 0;
}
