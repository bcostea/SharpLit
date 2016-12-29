/*--[transmute.c]--------------------------------------------------------------
 | Copyright (C) 2002, 2003 Dan A. Jackson
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

extern char * writingFilename;

/*
 | This file contains the routines to change DRM level and otherwise
 | write out a replacement .LIT file
 */
int writeat(void * v, U32 offset, U8 * ptr, int size);

int transmute_lit(lit_file * litfile, char * newlitfile, char * inscription)
{
    int     newlevel, status, i;
    U8      * drm_data = NULL;
    int     drm_size = 0;
    FILE    * fOut;
    lit_file outlit;

    writingFilename = newlitfile;
    fOut = fopen(newlitfile,"wb");
    if (!fOut) {
        lit_error(ERR_W|ERR_LIBC,"Unable to open \"%s\"!", newlitfile);
        return -1;
    }
    newlevel = 1;
    if (inscription)
    {
        newlevel = 3;
        drm_size = 2*(strlen(inscription)+1);
        drm_data = malloc(drm_size);
        if (!drm_data) { 
            lit_error(0,"Unable to allocate memory for inscription.");
            return E_LIT_OUT_OF_MEMORY;
        }
        memset(drm_data, 0, drm_size);
        for (i = 0; (size_t)i < strlen(inscription); i++) {
            drm_data[i*2] = inscription[i];
        }
    }
    status = lit_clone(litfile, &outlit);
    if (!status) {
        /* this is necessary because recrypt_section needs the callback
         | but it was on the original file. */
        outlit.drm5_callback = litfile->drm5_callback;
        outlit.drm5_data    = litfile->drm5_data;
        
        status = lit_change_drm_level(&outlit,newlevel,drm_data,drm_size);
    }
    if (!status) {
        outlit.file_pointer = (void *)fOut;
        outlit.writeat      = writeat;
        status = lit_write_to_file(&outlit);
    }

    if (drm_data) free(drm_data);
    lit_close(&outlit);
    return status;
}

int writeat(void * v, U32 offset, U8 * ptr, int size)
{
    FILE * f;
    int    r;

    f = (FILE *)v;
    if (!f) {
        lit_error(0,"No filehandle passed in!");
        return -1;
    }
    if (fseek(f, offset, SEEK_SET) != 0) {
        lit_error(ERR_LIBC|ERR_R,"fseek() failed.");
        return -1;
    }
    r = fwrite(ptr, 1, size, f);
    return r;
}

