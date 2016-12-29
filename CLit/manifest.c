/*--[manifest.c]--------------------------------------------------------------
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
#include "ext/litlib.h"
#include "manifest.h"
#include "utils.h"

/*-- Initialize the walk ------------*/
void mapping_init(mapping_state * m, manifest_type * pManifest)
{
    memset(m, 0, sizeof(mapping_state));
    m->p = pManifest;
    m->magic = 1984;
    m->group = -1;
}

/*-- Get each mapping, until NULL --*/
name_mapping * mapping_next(mapping_state * m)
{
    if ((!m) || (m->magic != 1984)) return NULL;

    if (m->index < m->max_index) 
    {
        m->map++;
        m->index++;
        return m->map;
    } else {
        m->group++;
        m->index = 0;
        m->map = NULL;
    }
    do {
        if (m->group >= NUM_MANIFEST_MAPS) {
            m->magic = 0;
            return NULL;
        }
        m->map = m->p->mappings[m->group];
        if (!m->map) m->group++;
    } while (!m->map);

    m->max_index = m->p->num_mappings[m->group];
    m->index = 1;
    return m->map;
}

int duplicate_manifest(manifest_type * pDest, manifest_type * pSource)
{
    int stage;
    int n;
    U8 * s;
    name_mapping * mapSrc, * mapDst;

    memset(pDest, 0, sizeof(manifest_type));

    for (stage = 0; stage < NUM_MANIFEST_MAPS; stage++) {
        if (!pSource->mappings[stage]) {
                pDest->mappings[stage] = NULL;
                continue;
        }
        mapSrc = pSource->mappings[stage];
        n = pSource->num_mappings[stage] * sizeof(name_mapping);
        mapDst = malloc(n);
        if (!mapDst) {
            lit_error(0,"Not enough memory for relative manifest.\n");
            lit_free_manifest(pDest);
            return -1;
        }
        memset(mapDst, 0, n);

        pDest->mappings[stage] = mapDst;
        for (n = 0; n < pSource->num_mappings[stage]; n++) {
            s = strmerge((char *)mapSrc->sOriginal, NULL, NULL);
            mapDst->sOriginal = s;
            if (!s) { lit_free_manifest(pDest); return -1;} 

            s = strmerge((char *)mapSrc->sInternal, NULL, NULL);
            mapDst->sInternal = s;
            if (!s) { lit_free_manifest(pDest); return -1;} 

            s = strmerge((char *)mapSrc->sType, NULL, NULL);
            mapDst->sType = s;
            if (!s) { lit_free_manifest(pDest); return -1;} 

            mapDst->offset = mapSrc->offset;

            mapSrc++;
            mapDst++;
            /* Allow freeing as we go */
            pDest->num_mappings[stage] = (n + 1);
        }
    }
    return 0;
}

/*
 | Here I want to make a path from "me" to "you", for each file of all types
 | in the manifest. 
 | "me" - the current HTML file
 | "you" - _EVERY_ other mapping.
 |
 | So, first I have to go back to a common directory.
 | Then, I'll be able to add the rest of "you" on.  And that's my link
 |
 */
int make_relative_manifest(U8 * file, manifest_type * pRelative, 
    manifest_type * pSource)
{
    int err;
    mapping_state  state;
    name_mapping   * map;

    utf8_iter       iterMe, iterYou;
    U8              *slashMe, *slashYou, *sRelative, *s;
    U32             c, d;

    err = duplicate_manifest(pRelative, pSource);
    if (err) {
        return err;
    }
    mapping_init(&state, pRelative);
    err = -300;
    while ((map = mapping_next(&state)) != NULL) {
        s = NULL;

        utf8_start(&iterMe,file);
        utf8_start(&iterYou, map->sOriginal);
        slashMe = file;
        slashYou = map->sOriginal;
        /* while the two paths are matched, skip up to the last slash */
        do {
            c = utf8_next(&iterMe);
            d = utf8_next(&iterYou);
            if (c != d)
                break;
            if ((c == '/') || (c =='\\')) { 
                slashMe = utf8_ptr(&iterMe); 
                slashYou = utf8_ptr(&iterYou); 
            }
        } while (c);

        utf8_start(&iterYou, slashYou);

        /* Create a new string, replacing each directory with "../"
         */
        s = NULL;

        sRelative = NULL;

        do {
            c = utf8_next(&iterMe);
            if ( (c == '/') || (c == '\\')) {
                if (c == '/') {
                    s = strmerge("../",sRelative, NULL);
                } else s = strmerge("..\\",sRelative, NULL);
                if (sRelative) free(sRelative);
                sRelative = s;
            }
        } while (c);
        if (sRelative) { 
            s = strmerge(sRelative,utf8_ptr(&iterYou),NULL);
            free(sRelative); sRelative = s;
        } else {
            sRelative = strmerge(utf8_ptr(&iterYou), NULL, NULL);
        }
        if (!sRelative) { 
            break;
        }
        free(map->sOriginal);
        map->sOriginal = sRelative;
        err = 0;  
    }
    if (err) {
        lit_free_manifest(pRelative);
    }
    else {
        display_manifest(pRelative);
    }
    return err; 
}

void display_manifest(manifest_type * pManifest)
{
#if 0 
    mapping_state   state;
    name_mapping    * map;
   
    printf("Manifest at %08lx\n", pManifest);
    mapping_init(&state, pManifest);
    while ((map = mapping_next(&state)) != NULL)
    {
        printf("--> Map %08lx\n", map);
        printf(".............. (%08lx) \"%s\" \n", map->sOriginal,
            map->sOriginal);
        printf(".............. (%08lx) \"%s\" \n", map->sInternal,
            map->sInternal);
        printf(".............. (%08lx) \"%s\" \n", map->sType, map->sType);
    }
    printf("[Done manifest dump]\n");
#endif
}


