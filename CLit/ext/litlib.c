/*--[litlib.c]-----------------------------------------------------------------
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
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif


/*****************************************************************************/
/*--[lit_close]----------------------------------------------------------------
 | This routine is responsible for freeing all memory used by the lit_file
 | structure (such as sections, entries, and so forth).
*/
int lit_close(lit_file * litfile)
{
    entry_type  *entry, *next;
    int         i;

    entry = litfile->entry;
    while (entry) {
        if ((entry->data_source == ENTRY_SOURCE_POINTER) &&
            (entry->data_pointer)) {
            free(entry->data_pointer);
        }
        next = entry->next;
        free(entry);
        entry = next;
    }
    litfile->entry = NULL;

    if (litfile->sections) {
        for (i = 0; i < litfile->num_sections; i++) {
            if ((litfile->sections[i].data_pointer) &&
                (litfile->sections[i].data_pointer != (U8 *)-1)) {
                free(litfile->sections[i].data_pointer);
                litfile->sections[i].data_pointer = NULL;
            }
        }
        free(litfile->sections);
        litfile->sections = NULL;
    }
    return 0;
}

/*****************************************************************************/
/*--[lit_read_from_file]-------------------------------------------------------
 |
 | This routine will read enough of the .LIT file structure for subsequent
 | routines to be able to access internal files. 
 | 
 | Currently this involves:
 |      1. Reading the headers
 |      2. Creating the entry list
 |      3. (As a side effect) reading nearly every section
 |      4. Determining the DRM level and verifying the decryption
*/
int lit_read_from_file(lit_file * litfile) 
{
    int     status;


    if (!litfile->readat) {
        lit_error(0,"Internal Error, no read callback supplied!");
        return E_LIT_BAD_STRUCT;
    }
    status = lit_i_read_headers(litfile);
    if (status) return status;

    status = lit_i_read_drm(litfile);
    return status;    
}


/*****************************************************************************/
/*--[lit_clone]----------------------------------------------------------------
 |
 | This routine clones a LIT file, creating a mirror image of the structure
 | with the associated files. 
 |
 | All necessary data pointers are allocated and copied over, so lit_close 
 | the new LIT file won't cause problems.
 |
 | HOWEVER, the entries themselves point to the OLD LIT file structure, and
 | closing the old litfile will severely reduce the usefulness of the new file. 
 | 
*/
int lit_clone(lit_file * litfile, lit_file * newlitfile)
{
    entry_type  *entry,*new,*newnext;

    memset(newlitfile, 0, sizeof(lit_file));

    newlitfile->drmlevel        = litfile->drmlevel;
    memcpy(&newlitfile->bookkey[0],&litfile->bookkey[0],8);
    memcpy(&newlitfile->header_guid[0],&litfile->header_guid[0],16);
    memcpy(&newlitfile->piece3_guid[0],&litfile->piece3_guid[0],16);
    memcpy(&newlitfile->piece4_guid[0],&litfile->piece4_guid[0],16);
    newlitfile->entry_chunklen  = litfile->entry_chunklen;
    newlitfile->count_chunklen  = litfile->count_chunklen;
    newlitfile->language_id     = litfile->language_id;
    newlitfile->creator_id      = litfile->creator_id;
    newlitfile->entry_unknown   = litfile->entry_unknown;
    newlitfile->count_unknown   = litfile->count_unknown;
    newlitfile->timestamp       = litfile->timestamp;

    entry = litfile->entry;
    newnext = NULL;
    while (entry) {
        new = malloc(sizeof(*entry)+entry->namelen+1);
        if (!new) { 
            lit_error(ERR_R|ERR_W,"Unable to clone directory structure -- "
                "out of memory!");
            return E_LIT_OUT_OF_MEMORY;
        }
        memcpy(new, entry, sizeof(*entry)+entry->namelen+1);
        new->data_source = ENTRY_SOURCE_LITFILE;
        new->data_pointer = (void *)litfile;
        if (!newnext) {
            newlitfile->entry = new;
            newnext = new;
        } 
        else {
            newnext->next = new;
            newnext = new;
        }
        entry = entry->next;
    }

    /* Reading the sections is a good idea here so code that uses the new
     | lit file can enumerate them. */
    return lit_i_read_sections(newlitfile);
}


/*--[lit_write_to_file]--------------------------------------------------------
 |
 | This is the externally visible routine to write the specified LIT file
 | to out via the supplied write function.
 | 
*/
int lit_write_to_file(lit_file * litfile) 
{
    int status;
    entry_type * entry;
    U8  * ptr = NULL; 
    int nbytes;

    if (!litfile->writeat) {
        lit_error(0,"Internal Error, no write callback supplied!");
        return E_LIT_BAD_STRUCT;
    }
    status = lit_i_write_headers(litfile);
    if (status) return status;

    entry = litfile->entry;
    while (entry)
    {
        if ((entry->section == 0) && (entry->size))
        {
            if ((entry->namelen == 1) && (entry->name == '/') ) {
                ;
            }
            else {
                status = lit_get_file(litfile,&entry->name,&ptr,&nbytes);
                if (status) return status;

                status = litfile->writeat(litfile->file_pointer,
                    (size_t)entry->offset + litfile->content_offset,
                    ptr, nbytes);
                free(ptr); 
                ptr = NULL;
                if (status != (int)entry->size) {
                    lit_error(ERR_W,
"Error copying files: Tried to write %d (Entry: %ld). Result = %d.",
                        nbytes, (size_t)entry->size, status);     
                    return E_LIT_WRITE_ERROR;
                }
            }
        }
        entry = entry->next;
    }
    return 0;
}

/*--[lit_get_file]-------------------------------------------------------------
 |
 | This routine is responsible for finding an internal filename. 
 | Ie, walk the linked list (created by read_lit_directory) and return 
 | the data (or just report on the presence if the ptr field is null).
 |
 | This routine will recursively call itself if another litfile is involved.
*/
int lit_get_file(lit_file * litfile, const char * name, U8 ** ptr, int * psize)
{
    entry_type  * entry;
    int         match, status;
    static int  depth = 0;
    U8          * newptr;
    section_type * section;

    if (depth > 10) {
        lit_error(ERR_R,"Recursed too deeply in lit_get_file!");
        return E_LIT_INTERNAL_ERROR;
    }
    if (ptr) *ptr = NULL;

    entry = litfile->entry;
    while (entry) {
        if (entry->namelen != (int)strlen(name)) match = -1;
        match = strncasecmp(&entry->name, name, entry->namelen);
        if ( (entry->namelen != (int)strlen(name)) && (!match)) {
            match = -1;
        }
        
        if (!match) break;

        entry = entry->next;
    }
    if (!entry) {
        /* This is only an error if the caller was interested in this 
         * file. Otherwise, its a warning (ie, looking for DRM3 entries in
         * a DRM1 litfile  */
        if (ptr) {
            lit_error(ERR_R,"Unable to find file \"%s\" in directory.",
                name);
        } 
        return E_LIT_FILE_NOT_FOUND;
    }
    if (psize) *psize = (int)(entry->size&0x7FFFFFFF);
    
    /* What are the contents of a 0-length file..? */
    if ((!ptr) || (!entry->size)) return 0; 


    /* Now, have to actually read that direntry */

    if ((entry->data_source == ENTRY_SOURCE_LITFILE) && 
        (entry->data_pointer)) {
        depth++;
        status = lit_get_file((lit_file *)entry->data_pointer,name,ptr,psize);
        depth--;
        return status;
    }
    if (entry->data_source == ENTRY_SOURCE_POINTER)
    {
        U8  * newptr;
        if (!entry->data_pointer) {
            lit_error(ERR_R,"Invalid pointer for file \"%s\".", name);
            return E_LIT_INTERNAL_ERROR;
        }
        newptr = malloc((size_t)entry->size);
        if (!newptr) {
            lit_error(ERR_R|ERR_LIBC,"Out of memory (malloc(%d) failed).",
                (size_t)entry->size);
            return E_LIT_OUT_OF_MEMORY;
        } 
        memcpy(newptr, entry->data_pointer, (size_t)entry->size);
        *ptr = newptr;
        return 0;
    }
    /* this should only leave reading from the current lit file */
    
    if (!entry->section) {
        if ((size_t)entry->size > litfile->filesize) {
            lit_error(ERR_R,
"Internal file had offset (%ld) outside filesize (%ld)!",
                (size_t)entry->size, litfile->filesize);
            return E_LIT_FORMAT_ERROR;
        }
        newptr = litfile->readat(litfile->file_pointer,
            litfile->content_offset+(size_t)entry->offset,(size_t)entry->size);
        if (!newptr) return E_LIT_READ_ERROR; 
        *ptr = newptr;
        return 0;
    } else {
        if (!litfile->sections) {
            status = lit_i_read_sections(litfile);
            if (status) return status;
        }
        if (entry->section > litfile->num_sections) {
            lit_error(ERR_R,"File \"%s\" has section %ld, greater then "
                "num_sections (%d)!\n",
                name, (U32)entry->section, litfile->num_sections);
            return E_LIT_FORMAT_ERROR;
        }

        section = &litfile->sections[entry->section];
 
        if (!section->data_pointer)
        {
            section->data_pointer = (U8 *)-1;
            status = lit_i_cache_section(litfile, section);
            if (status)  {
                section->data_pointer = NULL;
                return status;
            }
        }
        if ((entry->size+entry->offset) > (U64)section->size) {
            lit_error(ERR_R,
"Invalid Directory Entry (\"%.*s\", %d:%ld:%ld) \n"
"\t This is outside Section %d which is only %ld bytes long.",
                (int)entry->namelen, &entry->name, (int)entry->section,
                (U32)entry->offset, (U32)entry->size, (int)entry->section,
                (U32)section->size);
            return E_LIT_FORMAT_ERROR;
        }
        newptr = malloc((size_t)entry->size);
        if (!newptr) {
            lit_error(ERR_R|ERR_LIBC,"Malloc(%d) failed!", (size_t)entry->size);
            return E_LIT_OUT_OF_MEMORY;
        }
        memcpy(newptr,section->data_pointer+(size_t)entry->offset,
            (size_t)entry->size);
        *ptr = newptr;
    }
    return 0;
}


/*--[lit_remove_files]---------------------------------------------------------
 |
 | This removes all files that start with a specified prefix. 
 |
*/
int lit_remove_files(lit_file * litfile, const char * prefix)
{
    entry_type  * entry, * prev;
    int         match;

    entry = litfile->entry;
    prev = entry;
    if (!entry) return 0; 
    entry = entry->next;  /* Cannot remove the '/' entry! */

    while (entry) {
        match = strncasecmp(&entry->name,prefix, strlen(prefix));
        if (match == 0) {
            if ( (entry->data_source == ENTRY_SOURCE_POINTER) && 
                (entry->data_pointer)) {
                free(entry->data_pointer);
            }
            prev->next = entry->next;
            free(entry); 
            entry = prev;  
        }
        prev = entry;
        entry = entry->next;
    }
    return 0;
}


/*--[lit_put_file]-------------------------------------------------------------
 |
 | This puts a new file into the list.
 | This ONLY needs to know the size and the section -- offsets are taken 
 | care of elsewhere (depending on the section).
 |
 | Sections other than 0 won't work at the moment!
 | 
*/
int lit_put_file(lit_file * litfile, const char * name, U8 *ptr, int size, 
    int section)
{
    entry_type  * entry, * prev, * new;
    int         match;

    entry = litfile->entry;
    prev = entry;
    if (!entry) return 0;
    entry = entry->next;  /* Cannot remove the '/' entry! */

    while (entry) {
        if (entry->namelen != (int)strlen(name)) match = -1;
        match = strncasecmp(&entry->name, name, entry->namelen);
        if ( (entry->namelen != (int)strlen(name)) && (!match)) {
            match = -1;
        }

        if (!match) break;
        prev = entry;
        entry = entry->next;
    }
    if (entry) {
        if ((entry->data_source == ENTRY_SOURCE_POINTER) &&
            (entry->data_pointer))  free(entry->data_pointer); 
        entry->data_source = ENTRY_SOURCE_POINTER;
        entry->data_pointer = (void *)ptr;
        entry->size= size;
        entry->section = section;
        /* Everything else is the same! */        
    } else {
        new = malloc(sizeof(entry_type)+strlen(name));
        if (!new) {
            lit_error(ERR_R,"No memory for new entry."); 
            return E_LIT_OUT_OF_MEMORY;
        }
        new->data_source = ENTRY_SOURCE_POINTER;
        new->data_pointer = (void *)ptr;
        new->offset = 0;
        new->size = size;
        new->section = section;
        new->namelen = (int)strlen(name);
        strcpy(&new->name, name);
        new->next = prev->next;
        prev->next = new;
    }
    return 0;
}
