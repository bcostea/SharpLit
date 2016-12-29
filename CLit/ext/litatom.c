
/*--[litatom.c]--------------------------------------------------------------
 | Copyright (C) 2004 Digital Rights Software
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
#include <stdlib.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"

/*
 * This file contains routines to read and destroy atom (aka custom tag)
 * maps.
 */
/*
 | This is in the "/data/<internal name>/atom" internal file.
 |
 | Internal file format (no attributes)
 |  +-----+---+-------------+---+------------+ ... +-+-+
 |  |count|len|name 0 (len) |len|name 1 (len)|     |0|0|
 |  +-----+---+-------------+---+------------+ ... +-+-+
 | 
 | With attributes:
 |  +-+-+ 
 |  |0|0|  (end of structure)
 |  +-+-+  
 | becomes
 |  +-------+---------+-----------+    
 |  | count | length  | attr name | ... 
 |  +-------+---------+-----------+    
 | 
 | Strings are NOT zero terminated.
 | There is nothing after the last attribute name.
 */
/*--[lit_read_atoms]-----------------------------------------------------------
 |
 | This reads the atom list for a specified chunk of content. 
 | The absence of an atom list is not an error -- one may not be present.
 |  
*/
lit_atom_list * lit_read_atoms(lit_file * litfile, char * content_name)
{
    lit_atom_list * list;
    char * pathAtoms, *s;
    int status, nbytes;
    int nEntries, len, idx;
    U8 * p, * pAtoms;

    list = NULL;

    pathAtoms = lit_i_strmerge("/data/",content_name,"/atom",NULL);
    if (!pathAtoms) { return NULL; }
 
    pAtoms = NULL;
    /* See if the file exists first... */
    status = lit_get_file(litfile, pathAtoms, NULL, NULL);
    if (!status)
    {
        status = lit_get_file(litfile, pathAtoms, &pAtoms, &nbytes);
        if (nbytes <= 4) {
            free(pAtoms);
            pAtoms = NULL;
        }
    }
    p = pAtoms;
    free(pathAtoms); pathAtoms = NULL;

    if (!p) return NULL;

    nEntries = READ_U32(p);

    len = nEntries * sizeof(char *) + sizeof(lit_atom_list);
    list = (lit_atom_list *)malloc(len);
    if (!list) {
        lit_error(0,"Unable to malloc %d bytes for atoms.",len);
        free(pAtoms);
        return NULL;
    }
    memset( (void *)list, 0, len);

    list->num_atoms = nEntries;

    nbytes -= 4;
    p += 4;
      
    status = -1;
    idx = 0;
    while (idx < nEntries)
    {
        if (nbytes <= 1) break;

        len = *(p);
        if (!len) break;

        nbytes--;

        if (len > nbytes) { break;}

        s = (char *)malloc(len + 1);
        if (!s) { 
            lit_error(0,"Error allocating %d bytes of memory for strings.\n",
                len);
            break;
        }
        strncpy(s,p + 1, len);
        s[len] = '\0';

        list->atom_names[idx++] = s; 

        nbytes -= len;
        p += (len + 1);

        status = 0;
    }

    if (idx != nEntries)
    {
        lit_error(0,
"Warning - Unable to read custom tags - Expected %d entries, only read %d.\n",
                nEntries, idx);
        list->num_atoms = idx;
    }
    if (status)
    {
        lit_free_atoms(list);
        free(pAtoms);
        return NULL;
    } 
   
    /* No attribute map, all done here */
    if (nbytes < 4) { free(pAtoms); return list;}

    /* Now, start on the attribute map */
    nEntries = READ_U32(p);
    p += 4;
    nbytes -= 4;

    len = sizeof(lit_attr_map) * (nEntries + 1);
    list->attrmap = (lit_attr_map *)malloc(len);
    if (!list->attrmap)
    {
        /* Fatal error -- otherwise, those attributes can't be decoded! */
        lit_error(0,"Error allocating %d bytes for custom attributes.\n",len);
        lit_free_atoms(list);
        free(pAtoms);
        return NULL;
    }
    memset((void *)list->attrmap, 0, len);
    list->num_attrs = nEntries;

    idx = 0;
    while (idx < nEntries)
    {
        if (nbytes < 4) break;

        len = READ_U32(p);
        p += 4;
        nbytes -= 4;
       
        if (len > nbytes) { break;}

        s = (char *)malloc(len + 1);
        if (!s) {
            lit_error(0,"Error allocating %d bytes of memory for strings.\n",
                len);
            break;
        }
        strncpy(s,p, len);
        s[len] = '\0';

        list->attrmap[idx].name = s;
        list->attrmap[idx].id = idx + 1;
        idx++;

        nbytes -= len;
        p += len;
    }
    if (idx != nEntries)
    {
        lit_error(0,
"Warning - Unable to read custom attributes - Expected %d entries, read %d.\n",
            nEntries, idx);
        list->num_attrs = idx;
    }
    /* must add ending entry */
    list->attrmap[idx].name = NULL;
    list->attrmap[idx].id = 0;

    free(pAtoms); pAtoms = NULL;

    return list;
}


/*--[lit_free_atoms]---------------------------------------------------------
 |
 | This deallocates the memory associated with an atom list
 | 
*/
void lit_free_atoms(lit_atom_list * atoms)
{
    int i;
    if (atoms)
    {
        if (atoms->attrmap) 
        {
            for (i = 0; i < atoms->num_attrs; i++) 
                free((void *)atoms->attrmap[i].name);
            free(atoms->attrmap);
        }
        for (i = 0; i < atoms->num_atoms; i++)
            free(atoms->atom_names[i]);
        free(atoms);
    }
}

