/*--[litmanifest.c]------------------------------------------------------------
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
/* 
 | This file contains the routines for processing the "/manifest" file
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"

const char * manifest_string = "/manifest";

/*--[lit_free_manifest]--------------------------------------------------------
 | 
 | Here all memory used by a manifest structure is freed.
 | 
 */
int lit_free_manifest(manifest_type * pmanifest)
{
    int i, j;

    for (j = 0; j < 4; j++)
    {
        name_mapping * map;

        if (!pmanifest->mappings[j]) continue;
        map = pmanifest->mappings[j];
        for (i = 0; i < pmanifest->num_mappings[j]; i++)
        {
            if ((map) && (map->sOriginal))
                free(map->sOriginal);
            if ((map) && (map->sInternal))
                free(map->sInternal);
            if ((map) && (map->sType))
                free(map->sType);
            map++;
        }
        free(pmanifest->mappings[j]);
        pmanifest->mappings[j] = NULL;
        pmanifest->num_mappings[j] = 0;
    }
    return 0;
}

/**** "/manifest" format *****************************************************
 
 Lengths APPEAR to be single bytes. These MAY be ENCINTs, but that has not yet 
 been tested (nor have any been seen). All lengths are in UTF8 ELEMENTS, _NOT_ 
 in bytes! 

     BYTE   DirectoryLength(*)
     BYTES  DirectoryName
 
     DWORD  Number of entries in map 0 (HTML files in the spine?)
     For each entry
     DWORD  Offset into section 
     BYTE   ID Length(*)
     BYTES  ID 
     BYTE   HREF Length(*)
     BYTES  HREF
     BYTE   ContentType Length(*)
     BYTES  ContentType

     And so forth, for map 1, 2, 3
     Map 1 - These are additional HTML files (Not in the spine?) 
     Map 2 - Any CSS files
     Map 3 - Image files

 ***************************************************************************/

/*--[lit_read_manifest]--------------------------------------------------------
 |
 | This routine will parse the "/manifest" file, which contains a record of
 | the individual files which made up the .LIT files -- their original names,
 | the names used internally, and the type.
 |
*/
int lit_read_manifest(lit_file * litfile, manifest_type * pmanifest)
{
    U8      *pManifest, *p, * pEnd;
    int     size;
    int     num_files, i;
    int     offset, sizeManifest;
    int     state, status;
    U8      * sOriginal = NULL, * sInternal = NULL, *sContentType = NULL;
    name_mapping * map = NULL;

    status = lit_get_file(litfile,manifest_string,&pManifest,&sizeManifest);
    if (status) return status;

    if (!pmanifest) { return 0; }
    memset(pmanifest, 0, sizeof(manifest_type));

    p = pManifest;
    pEnd = p + sizeManifest;

    while ((pEnd - p) >= 1) {
        /* First, the directory */
        size = *(p++);

        if (!size) break;
        if ((p+size) > pEnd)    goto out_of_range;

        /* I ignore the directory information here, as it is useless for 
         | any purpose I can think of.  
         */
        p += size;

        for (state = 0; state < NUM_MANIFEST_MAPS; state++)
        {
            /* 0 -- HTML files in spine
             * 1 -- HTML files not in spine (??)
             * 2 -- CSS files
             * 3 -- Image files
             */
            size = 4;

            if ((p+size) > pEnd) goto out_of_range;
            num_files = READ_INT32(p);

            p += 4;

            if (!num_files) continue;

            map = malloc(sizeof(name_mapping)*num_files);
            if (!map) {
                lit_error(ERR_LIBC|ERR_R,"malloc(%d) failed for name_mapping!",
                    sizeof(name_mapping)*num_files); 
                free(pManifest);
                return E_LIT_OUT_OF_MEMORY;
            }

            for (i = 0; i < num_files; i++)
            {
                size = 5;
                if ((p+size) > pEnd) goto out_of_range;
                offset = READ_U32(p);
                p+=4;

                /*
                 . This is the internal reference to this file
                 */
                sInternal = lit_i_read_utf8_string(p,(pEnd - p), &size);
                if (!sInternal) goto error;
                p += size;

                /*
                 . Read source filename
                 */
                sOriginal = lit_i_read_utf8_string(p,(pEnd - p), &size);
                if (!sOriginal) goto error;
                p += size;

                /* This is the MIME type, with a trailing 0! */
                /* The MIME type should NEVER be UTF8, but why not.. */
                sContentType = lit_i_read_utf8_string(p,(pEnd - p), &size);
                if (!sContentType) goto error;
                p += size;
#if 0
                printf(".....  %s \"%s\"-- %s\n", sOriginal,
                    sContentType, sInternal ); 
#endif

                p++;
                /* I pray that was a asciiz string and there is not an
                 * optional fourth parameter here. */

                map[i].sOriginal = sOriginal;
                map[i].sInternal = sInternal;
                map[i].sType = sContentType;
                map[i].offset = offset;
                sOriginal = sInternal = sContentType = NULL;
            }
            pmanifest->mappings[state] = map;
            map = NULL;
            pmanifest->num_mappings[state] = num_files;
        }

    }
    free(pManifest); 
    pManifest = NULL;

    if ((pEnd - p)) {
        lit_error(ERR_R, "Data left over in \"/manifest\" (%d bytes)!\n",
            (pEnd - p));
        return E_LIT_FORMAT_ERROR;
    }

    return 0;
out_of_range:
    lit_error(ERR_R,
"LIT Format error: truncated \"/manifest\" (%d out of %d, reading %d)!\n",
                (p - pManifest), sizeManifest, size);
error:
    if (pManifest) free(pManifest);
    if (map) free(map);
    if (sOriginal) free(sOriginal);
    if (sInternal) free(sInternal);
    if (sContentType) free(sContentType);
    return -1;
}


/*--[lit_lookup_mapping]-------------------------------------------------------
 |
 | This walks the manifest mappings to convert the internal name back into
 | the external name.
*/
char * lit_lookup_mapping(manifest_type * pmanifest,U8 * s,int size)
{
    int i, j;

    for (j = 0; j < NUM_MANIFEST_MAPS; j++)
    {
        name_mapping * map;

        if (!pmanifest->mappings[j]) continue;
        map = pmanifest->mappings[j];
        for (i = 0; i < pmanifest->num_mappings[j]; i++)
        {
            if (strncmp(s, map->sInternal, size) == 0)
                return map->sOriginal;
            map++;
        }
    }
    return NULL;
}

