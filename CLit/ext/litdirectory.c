
/*--[litdirectory.c]-----------------------------------------------------------
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
#include <stdlib.h>
#include <string.h>
#include "litlib.h"
#include "litinternal.h"

/* This file contains routines for reading and writing to directory 
 | structures. 
 */


/* ... see comment further down in the file .... */

typedef struct aolx_struct {
    U8      * pBase;
    int     total_size;
    int     index;
    int     entries;
    int     remaining;
} aolx_struct;

typedef struct ifcm_struct {
    U8      * pData;
    int     total_size;
    int     chunk_size;
    int     chunk;
    aolx_struct AOLL;
    aolx_struct AOLI;
    int     aoll_chunk;
    int     aoli_chunk;
} ifcm_struct;


static int aoli_add_name(aolx_struct * ,entry_type * , int );
static void aoli_finish(aolx_struct * , int );
static int aoll_add_entry(aolx_struct * , entry_type * );
static int aoll_add_index(aolx_struct * ,int ,int ,int );
static void aoll_finish(aolx_struct * ,int , int , int , int );
static int ifcm_free(ifcm_struct * );
static int ifcm_init(ifcm_struct *, int , U32 );
static int ifcm_new_aolx(ifcm_struct *, U32 );
static void read_encint(U8 **handle, int *remaining, U64 *value);
static entry_type * read_entry(U8 **handle, int *remaining);
static int write_encint(U8 * p, int index,int total, U64 x);


/*--[read_encint]--------------------------------------------------------------
 |
 | This reads an encoded integer from the data stream.
 |   - ENCINT data type is a variable length integer, where the MSB
 |   indicates if another byte is needed and the lower 7 bits represent
 |   the data.
 |
 |   For Example:
 |                          +-+-------------+
 |       0x7F stays 0x7F    |0|1 1 1 1 1 1 1|
 |                          +-+-------------+
 |
 |                             +-+-------------+   +-+-------------+
 |      0xFE becomes 0xFF 0x00 |1|1 1 1 1 1 1 1|   |0|0 0 0 0 0 0 0|
 |                             +-+-------------+   +-+-------------+
 | 
*/
void read_encint(U8 **handle, int *remaining, U64 *value)
{
    U8   b, *p;

    p = *handle;
    *value = 0;
    while (*remaining > 0) {
        b = *(p++);
        (*remaining)--;
        (*value) <<= 7;
        (*value) |= (b & 0x7f);
        if (!(b & 0x80)) break;
    }
    *handle = p;
}

/*--[write_encint]-------------------------------------------------------------
 | 
 | Store an encoded integer into a location.
*/
int write_encint(U8 * p,int index,int total,U64 x)
{
    int     len,i;
    U64     mask;
    
    len = 0;
    mask = 0x7f;
    while (x > mask) { len++; mask = (mask << 7)|0x7f;  }
    len++;
    if (len > (total - index)) return -1; 
    
    for (i = 1; i < len; i++)
    {
        *(p+index) = 0x80 + (U8)((x & mask) >> ((len - i) * 7));
        mask >>= 7;
        index++;
    }
    *(p+index) = (U8)((x & mask) &0xff);
    return len;    
}


/*--[read_entry]---------------------------------------------------------------
 |
 | This routine reads an entry given a pointer and the number of bytes 
 | remaining.
 |
 | A directory entry is:
 |   ENCODED_INTEGER      name_length
 |   BYTES                name
 |   ENCODED_INTEGER      section
 |   ENCODED_INTEGER      offset
 |   ENCODED_INTEGER      size
 |
*/
entry_type * read_entry(U8 **handle, int *remaining)
{
    int    namelen;
    U64    long_namelen;
    U8     * nameptr;
    entry_type * entry;

    read_encint(handle, remaining, &long_namelen);
    namelen = (int)(long_namelen & 0x7fffffff);
    if (long_namelen != namelen) {
        lit_error(ERR_R,"Directory entry had 64-bit name length!");
        return NULL;
    }
    nameptr = *handle;
    if ((namelen + 3) <= *remaining) {
        (*handle)+= namelen;
        *remaining -= namelen;
    } else {
        lit_error(ERR_R,"Read past end of directory chunk!");
        return NULL;
    }

    entry = malloc(sizeof(entry_type)+namelen + 1);
    if (!entry) {
        lit_error(ERR_LIBC,"malloc(%d) failed!", sizeof(entry_type)+namelen+1);
        return NULL;
    }
    entry->next = NULL;
    entry->data_source = ENTRY_SOURCE_LITFILE;
    entry->data_pointer = NULL;
    entry->namelen = namelen;
    memcpy(&entry->name, nameptr, namelen);
    /* Avoid warnings about going past the end of an array */
    *( (U8 *)(&entry->name) + namelen) = '\0'; 
    read_encint(handle,remaining,&entry->section);
    read_encint(handle,remaining,&entry->offset);
    read_encint(handle,remaining,&entry->size);
    return entry;
}

/*--[write_entry]--------------------------------------------------------------
 | 
 | Similar to read entry above.  
 |
*/
int write_entry(U8 * p,int base_index,int total,entry_type * entry)
{
    int len,index = base_index;
    U32 q;

    q = entry->namelen; /* GCC gives garbage in high Dword without this */
    len = write_encint(p, index, total, q);
    if (len >= 0) {
        index += len;
        len = entry->namelen;
        if ( (index+len) < total) {
            memcpy(p+index, &entry->name,len);
        } else  len = -1;
    }
    if (len >= 0) {
        index += len;
        len = write_encint(p, index, total, entry->section);
    }
    if (len >= 0) {
        index += len;
        len = write_encint(p, index, total, entry->offset);
    }
    if (len >= 0) {
        index += len;
        len = write_encint(p, index, total, entry->size);
    }
    if (len >= 0) index += len;
    
    if (len < 0) {
        if ((index - base_index) > 0)
            memset(p+base_index, 0, index - base_index);
        return E_LIT_NOT_ENOUGH_BYTES;
    }
    return (index - base_index);
}

/*** DIRECTORY STRUCTURES *****************************************************

    +-----------------------------+
    |                             |
    | Directory header            |
    |                             |
    +-----------------------------+
    |                             |
    | Directory Entries ---->     |
    |                             |
    +-----------------------------+
    |                             |
    |                             |
    |    <---- QuickRef Entries   |
    +-----------------------------+

 +-------------------------------+--------------------------------+
 | Block Tag "IFCM"              | Version (typically 1)          |
 +-------------------------------+--------------------------------+
 | "Chunk Size"                  | Unknown (0x100000 or 0x20000)  |
 +-------------------------------+--------------------------------+
 | 0xFFFFFFFF 0xFFFFFFFF (Unknown)                                |
 +-------------------------------+--------------------------------+
 | Number of chunks                                               |
 +----------------------------------------------------------------+


 +-------------------------------+--------------------------------+
 | Second Tag "AOLL"             | Number of bytes after entries  |
 +-------------------------------+--------------------------------+
 | Current chunk id (0 based)                                     |
 +----------------------------------------------------------------+
 | Previous AOLL chunk id                                         |
 +----------------------------------------------------------------+
 | Next AOLL chunk id (0 based)                                   |
 +----------------------------------------------------------------+
 | Entries so far                                                 |
 +----------------------------------------------------------------+
 | 1 (Distance in chunks to next)                                 |
 +----------------------------------------------------------------+
 | Directory entries
 +-------------------------------.....

******************************************************************************/
#define IFCM_TAG        0x4D434649
#define AOLL_TAG        0x4C4C4F41
#define AOLI_TAG        0x494C4F41


/*--[lit_i_read_directory]-----------------------------------------------------
 |
 | This reads the directory chunks from the piece and converts the list into
 | entry elements in the lit_file structure.
 | 
*/
int lit_i_read_directory(lit_file * litfile, U8 * piece, int piece_size)
{
    U8      * p = NULL;
    int     nEntries, nRemaining, nChunks, chunk, idx, sizeChunk;
    entry_type * entry, * prev;

    if (!piece || (READ_U32(piece) != IFCM_TAG)) {
        lit_error(ERR_R,
           "Header Piece #3 is not the main directory! (TAG=%08lx)",
           (piece)?(READ_U32(piece)):0);
        return E_LIT_FORMAT_ERROR;
    }
    sizeChunk = READ_INT32(piece + 8);
    nChunks = READ_INT32(piece + 24);

    if ((32 + (nChunks * sizeChunk)) !=  piece_size) {
        lit_error(ERR_R, "IFCM HEADER (%d chunks of %d bytes) != %d total.",
        nChunks, sizeChunk, piece_size - 32);
        return E_LIT_FORMAT_ERROR;
    }
    prev = NULL;
    for (chunk = 0; chunk < nChunks; chunk++)
    {
        p = piece + 32 + (chunk * sizeChunk);

        /* This can either be AOLL or AOLI.
         * AOLI isn't useful for reading */
        if (READ_U32(p) != AOLL_TAG)  continue;

        nRemaining = READ_INT32(p + 4);
        if (nRemaining >= sizeChunk) {
            lit_error(ERR_R,
"AOLL remaining count is NEGATIVE! (%d of %d) %x\n",
            (int)nRemaining, (int)sizeChunk, (int)nRemaining);
            return E_LIT_FORMAT_ERROR;
        }
        nRemaining = sizeChunk - (nRemaining + 48);

        nEntries = READ_U16(p + sizeChunk - 2);

        /* Sometimes, the nEntries doesn't get written. When this happens,
         * I don't know how many to read.  Fortunately, there is "nRemaining",
         * and if everything is working fine, read_entry will consume JUST 
         * enough bytes */
        if (!nEntries) nEntries = 65535;

        p += 48;
        if (nRemaining < 0) return E_LIT_FORMAT_ERROR;
        for (idx = 0; idx < nEntries; idx ++) {
            if (nRemaining <= 0) break;
            entry = read_entry(&p, &nRemaining);
            if (!entry) {
                return E_LIT_FORMAT_ERROR;
            }
            if (!prev) {
                litfile->entry = entry;
                prev = entry;
            } else {
                prev->next = entry;
                prev = entry;
            }    
        }
    }
    return 0;
}

/*--[lit_i_free_dir_type]------------------------------------------------------
 |
 | releases memory associated with a dir_type
 | 
*/
void lit_i_free_dir_type(dir_type * dirtype)
{
    if (dirtype->entry_ptr) free(dirtype->entry_ptr);
    if (dirtype->count_ptr) free(dirtype->count_ptr);
    memset(dirtype, 0, sizeof(dir_type));
}


/*--[aoli_add_name]------------------------------------------------------------
 |
*/
int aoli_add_name(aolx_struct * aolx,entry_type * entry, int chunk)
{
    int  len;
    int  base_index;
    U64  q;

    base_index = aolx->index;
    if ((aolx->entries % 5) == 0) {
        aolx->remaining -= 2; 
        WRITE_U32(aolx->pBase+aolx->remaining, aolx->index - 48);
    }
    q = entry->namelen;     
    len = write_encint(aolx->pBase,aolx->index,aolx->remaining, q);
    if (len >= 0) {
        aolx->index += len;
        len = entry->namelen;
        if ( (aolx->index+len) < aolx->remaining) {
            memcpy(aolx->pBase+aolx->index, &entry->name,len);
        } else  len = -1;
    }
    if (len >= 0) {
        aolx->index += len;
        q = chunk;
        len = write_encint(aolx->pBase,aolx->index,aolx->remaining, q);
    }
    if (len >= 0) aolx->index += len;
    if (len < 0) {
        if ((aolx->index - base_index) > 0)
            memset(aolx->pBase+base_index, 0, aolx->index - base_index);
        return E_LIT_NOT_ENOUGH_SPACE;
    }
    aolx->entries++;
    return 0;
}

/*--[aoli_finish]--------------------------------------------------------------
 |
*/
void aoli_finish(aolx_struct * aolx, int param)
{
    WRITE_U16(aolx->pBase + (aolx->total_size - 2), aolx->entries);  
    WRITE_U32(aolx->pBase + 4, aolx->total_size - aolx->index);
    WRITE_U32(aolx->pBase + 8, param);
}

/*--[aoll_add_entry]-----------------------------------------------------------
 |
*/
int aoll_add_entry(aolx_struct * aolx, entry_type * entry) {
    int     nbytes;

    if ((aolx->entries % 5) == 0) {
        aolx->remaining -= 2; 
        WRITE_U16(aolx->pBase+aolx->remaining, aolx->index - 48);
    }
    nbytes = write_entry(aolx->pBase,aolx->index,aolx->remaining,entry);
    if (nbytes < 0) {
        return nbytes;
    }
    aolx->index += nbytes;
    aolx->entries++;
    return 0;
}

/*--[aoll_add_index]-----------------------------------------------------------
 |
*/
int aoll_add_index(aolx_struct * aolx,int so_far,int current,int chunk)
{
    int  len;
    int  base_index;
    U64  q;

    base_index = aolx->index;
    if ((aolx->entries % 5) == 0) {
        aolx->remaining -= 2; 
        WRITE_U16(aolx->pBase+aolx->remaining, aolx->index - 48);
    }
    q = so_far;
    len = write_encint(aolx->pBase,aolx->index,aolx->remaining,q);
    if (len >= 0) {
        aolx->index += len;
        q = current;
        len = write_encint(aolx->pBase,aolx->index,aolx->remaining, q);
    }
    if (len >= 0) {
        aolx->index += len;
        q = chunk;
        len = write_encint(aolx->pBase,aolx->index,aolx->remaining, q);
    }
    if (len >= 0) aolx->index += len;
    if (len < 0) {
        if ((aolx->index - base_index) > 0)
            memset(aolx->pBase+base_index, 0, aolx->index - base_index);
        return E_LIT_NOT_ENOUGH_SPACE;
    }
    aolx->entries++;
    return 0;
}

/*--[aoll_finish]--------------------------------------------------------------
 |
*/
void aoll_finish(aolx_struct * aolx,
    int entries_so_far, int prev, int cur, int next)
{
    WRITE_U16(aolx->pBase + (aolx->total_size - 2), aolx->entries);  
    WRITE_U32(aolx->pBase + 4, aolx->total_size - aolx->index);

    if (cur >= 0) {
        WRITE_U32(aolx->pBase +  8, cur);
        WRITE_U32(aolx->pBase + 12, 0);
    } else  {
        WRITE_U32(aolx->pBase +  8, 0xFFFFFFFF);
        WRITE_U32(aolx->pBase + 12, 0xFFFFFFFF);
    }
    if (prev >= 0) {
        WRITE_U32(aolx->pBase + 16, prev);
        WRITE_U32(aolx->pBase + 20, 0);
    } else  {
        WRITE_U32(aolx->pBase + 16, 0xFFFFFFFF);
        WRITE_U32(aolx->pBase + 20, 0xFFFFFFFF);
    }

    if (next >= 0) {
        WRITE_U32(aolx->pBase + 24, next);
        WRITE_U32(aolx->pBase + 28, 0);
    } else  {
        WRITE_U32(aolx->pBase + 24, 0xFFFFFFFF);
        WRITE_U32(aolx->pBase + 28, 0xFFFFFFFF);
    }

    WRITE_U32(aolx->pBase + 32, entries_so_far);

    if (next-cur > 0)
    {
        WRITE_U32(aolx->pBase + 40, next-cur);
    }
    else
    {
        WRITE_U32(aolx->pBase + 40, 1);
    }
}

/*--[lit_i_make_directories]---------------------------------------------------
 |
 | This creates new directory structures and fills in the dir_type structure
 | with the created header information. 
 |
*/
int lit_i_make_directories(lit_file * litfile, dir_type * dirtype)
{
    entry_type      *entry;
    int             r, entries_so_far, last_chunk, next_chunk, chunk;
    int             has_aoli;
    ifcm_struct     ifcmIndex, ifcmDir;
    U32             total_content_length;

    if ((!litfile->entry_chunklen) || (!litfile->count_chunklen)) {
        lit_error(ERR_W,"Invalid chunk lengths when making directories!\n");
        return E_LIT_BAD_STRUCT;
    }
    r = ifcm_init(&ifcmIndex, litfile->count_chunklen, 
        litfile->count_unknown);
    if (!r) r = ifcm_init(&ifcmDir, litfile->entry_chunklen, 
        litfile->entry_unknown);
    if (r) {
        lit_error(ERR_W,"Error (%d) initializing IFCM structures.", r);
        goto bad;
    }
    r = ifcm_new_aolx(&ifcmDir, AOLL_TAG);
    if (!r) r = ifcm_new_aolx(&ifcmIndex, AOLL_TAG);
    if (r) {
        lit_error(ERR_W,"Error (%d) creating AOLL chunks.", r);
        goto bad;
    }
    has_aoli = 0;
    last_chunk = -1;
    chunk = 0;
    next_chunk = chunk+1;
    total_content_length = 0;
    entries_so_far = 0;
    entry = litfile->entry; 
    while (entry)
    {
        if (((entry->namelen > 1) || (entry->name != '/')) && 
            (entry->section == 0) ) {
            entry->offset = total_content_length; 
            total_content_length += (size_t)entry->size;
        }
        r = aoll_add_entry(&ifcmDir.AOLL, entry);
        if (r)
        {
            if (!has_aoli) {
                r = ifcm_new_aolx(&ifcmDir,AOLI_TAG);
                if (r) {
                    lit_error(ERR_W,"Error (%d) while making first AOLI.",r);
                    goto bad;
                }
                /* This points to the first file */
                r = aoli_add_name(&ifcmDir.AOLI,litfile->entry, 0);
                if (r) {
                    /* WTF!
                     * Allocate a new AOLI here?
                     * Are the AOLI links to/from this? 
                     */
                    lit_error(ERR_W,
"Too many files in AOLI, and I can't create a second AOLI block!\n"); 
                    goto bad;
                }
                has_aoli++;
                next_chunk++;
            }
            aoll_finish(&ifcmDir.AOLL,entries_so_far,last_chunk,
                chunk,next_chunk);
            r = aoll_add_index(&ifcmIndex.AOLL,entries_so_far,
                ifcmDir.AOLL.entries,chunk);
            if (r) { 
                lit_error(ERR_W,
"Too many count entries in AOLL - This case should not happen!\n");
                goto bad;
            }
            last_chunk = chunk;
            chunk = next_chunk;
            next_chunk++;

            entries_so_far += ifcmDir.AOLL.entries;

            r = ifcm_new_aolx(&ifcmDir,AOLL_TAG);
            if (r) {
                lit_error(ERR_W,"Error (%d) making additional AOLL.", r);
                goto bad;
            }

            r = aoli_add_name(&ifcmDir.AOLI,entry, chunk);
            if (r) { 
                lit_error(ERR_W,
"Unable to add first entry of new chunk to AOLI.\n");
                goto bad;
            }

            /* Should work this time */
            r = aoll_add_entry(&ifcmDir.AOLL, entry);
            if (r) {
                lit_error(ERR_W,"Writing to new directory failed!");
                goto bad;
            }
        }
        entry = entry->next;
    }
    aoll_finish(&ifcmDir.AOLL, entries_so_far, last_chunk, chunk, -1);
    aoll_add_index(&ifcmIndex.AOLL,entries_so_far,ifcmDir.AOLL.entries,chunk);
    if (has_aoli) {
        aoli_finish(&ifcmDir.AOLI, 1);
    }
    aoll_finish(&ifcmIndex.AOLL, 0, -1, 0, -1);

    if (!dirtype) return 0;

    /* Note - last_chunk only works BECAUSE I put the AOLI in the middle
     | (following LITGEN.DLL) and not at the end (which AEBIN does). */
    dirtype->entry_ptr = ifcmDir.pData;
    dirtype->entry_size = ifcmDir.total_size;
    dirtype->entry_last_chunk = ifcmDir.chunk;
    dirtype->count_ptr = ifcmIndex.pData;
    dirtype->count_size = ifcmIndex.total_size;
    dirtype->count_last_chunk = ifcmIndex.chunk;
    dirtype->num_entries = entries_so_far;
    dirtype->num_counts = ifcmIndex.AOLL.entries;
    dirtype->total_content_size = total_content_length;
    /* Again.. Assuming one AOLI index at a particular spot! */
    if (has_aoli) { dirtype->entry_aoli_idx = 1; }
    else { dirtype->entry_aoli_idx = -1; }

    return 0; 
bad:
    ifcm_free(&ifcmDir);
    ifcm_free(&ifcmIndex);
    return r;
}

/*--[ifcm_free]----------------------------------------------------------------
 |
*/
int ifcm_free(ifcm_struct * ifcm)
{
    if (ifcm->pData) free(ifcm->pData);
    return 0;
}

/*--[ifcm_init]----------------------------------------------------------------
 |
*/
int ifcm_init(ifcm_struct * ifcm, int chunk_size, U32 param)
{
    unsigned char       *p;

    p = malloc(chunk_size + 32);
    if (!p) return E_LIT_OUT_OF_MEMORY;
    WRITE_U32(p,    IFCM_TAG);
    WRITE_U32(p+ 4, 1);
    WRITE_U32(p+ 8, chunk_size);
    WRITE_U32(p+12, param);
    WRITE_U32(p+16, 0xFFFFFFFF );
    WRITE_U32(p+20, 0xFFFFFFFF );
    WRITE_U32(p+24, 1 );
    WRITE_U32(p+28, 0 );

    ifcm->total_size = chunk_size + 32;
    ifcm->chunk_size = chunk_size;
    ifcm->chunk      = 0;
    ifcm->pData      = p;

    ifcm->aoll_chunk = -1;
    ifcm->aoli_chunk = -1;
    memset(&ifcm->AOLL, 0, sizeof(aolx_struct));
    memset(&ifcm->AOLI, 0, sizeof(aolx_struct));
    return 0;
}

/*--[ifcm_new_aolx]------------------------------------------------------------
 |
 | the IFCM (outer block) reallocates itself every time in needs to 
 | add on another AOLx structure.
 | 
 | Fortunately, once an AOLX structure is done, it never needs to be 
 | directly accessed again.
*/
int ifcm_new_aolx(ifcm_struct * ifcm, U32 tag)
{
    aolx_struct * aolx;
    U8          * pBase;

    if (ifcm->chunk > 0) {
        ifcm->pData = (unsigned char *)realloc(ifcm->pData,
            (ifcm->chunk_size * (ifcm->chunk + 1)) + 32);
        if (!ifcm->pData) {
            lit_error(ERR_W,"Unable to realloc from %d to %d.\n",
                (ifcm->chunk_size * ifcm->chunk) + 32, 
                (ifcm->chunk_size * (ifcm->chunk + 1)) + 32);
            return E_LIT_OUT_OF_MEMORY;
        }
        ifcm->total_size = (ifcm->chunk_size * (ifcm->chunk + 1)) + 32;
    }
    pBase = ifcm->pData + (ifcm->chunk_size * ifcm->chunk) + 32;

    if (tag == AOLL_TAG) {
        aolx = &ifcm->AOLL;
        memset(pBase+16, 0xff, 16);
        memset(pBase+32,    0, 16); 
        aolx->index = 48;
        ifcm->aoll_chunk = ifcm->chunk;
    } else { /* AOLI */
        aolx = &ifcm->AOLI;
        aolx->index = 16;
        ifcm->aoli_chunk = ifcm->chunk;
    }
    if (ifcm->aoli_chunk >= 0)
        ifcm->AOLI.pBase = ifcm->pData+32+(ifcm->chunk_size*ifcm->aoli_chunk); 
    if (ifcm->aoll_chunk >= 0)
        ifcm->AOLL.pBase = ifcm->pData+32+(ifcm->chunk_size*ifcm->aoll_chunk); 
    memset(pBase,       0, 16);
    WRITE_U32(pBase, tag);
    memset(pBase + aolx->index, 0, ifcm->chunk_size - aolx->index);

    WRITE_U32(ifcm->pData + 24, ifcm->chunk + 1);

    aolx->total_size = ifcm->chunk_size;
    aolx->entries = 0;
    /* Reserve space for total count. */
    aolx->remaining = ifcm->chunk_size - 2;
    ifcm->chunk++;

    return 0;
}
