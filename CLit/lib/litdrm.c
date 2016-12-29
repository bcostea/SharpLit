/*****************************************************************************/
/*--[litdrm.c]-----------------------------------------------------------------
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
#include <time.h>
#include "litlib.h"
#include "litinternal.h"
#include "d3des.h"
#include "sha.h"

static int calculate_deskey(lit_file *, U8 key[8]);
static void hash_bytes(SHA_CTX * pctx, U8 * pdata, int length, int padding);
static void random_data(U8 * pData, int len);

#define SEALED_SIZE 16

/* 
 | This file contains routines to deal with the two standard DRM levels.
 | Those levels are
 |    DRM1 - "Sealed"
 |    	"/DRMStorage/DRMSealed" contains a DES key that is encrypted with
 |      a DES key generated from "/meta" and "/DRMStorage/DRMSource"
 |    DRM3 - "Inscribed"
 |    	"/DRMStorage/DRMSealed" contains a DES key that is encrypted with
 |      a DES key generated from "/meta","/DRMStorage/DRMSource", and
 | 	"/DRMStorage/DRMBookplate" (the inscribed text in Unicode).
 |
 | "/DRMStorage/ValidationStream" should always be "MSReader" if the 
 | decryption works.
 */

static const char * drmsource_string = "/DRMStorage/DRMSource";
static const char * sealed_string = "/DRMStorage/DRMSealed";
static const char * bookplate_string = "/DRMStorage/DRMBookplate";
static const char * validation_string = "/DRMStorage/ValidationStream";
static const char * license_string = "/DRMStorage/Licenses/EUL";
static const char * license_path_string = "/DRMStorage/Licenses";
static const char * meta_string = "/meta";
static const char * msreader_string = "MSReader";


/*--[lit_i_read_drm]-----------------------------------------------------------
 |
 | This routine checks for the presence of DRM and initializes the DRM level
 | and bookkey appropriately.
 | 
*/
int lit_i_read_drm(lit_file * litfile)
{
    U8      * ptr, tmpkey[8];
    int     size, ofs;
    int     status;

    status = lit_get_file(litfile, meta_string, NULL, NULL);
    if (status) {
        lit_error(ERR_R,"Bad LIT directory - no \"%s\" file!",meta_string);
        return status;
    }
    litfile->drmlevel = 0;
    status = lit_get_file(litfile, license_string, NULL, NULL);
    if (!status) litfile->drmlevel = 5;
    if (status) {
        status = lit_get_file(litfile,bookplate_string, NULL, NULL);
        if (!status) litfile->drmlevel = 3;
    }
    if (status) {
        status = lit_get_file(litfile,sealed_string, NULL, NULL);
        if (!status) litfile->drmlevel = 1;
    }
    if (!litfile->drmlevel) return 0;

    if (litfile->drmlevel < 5) {
        status = calculate_deskey(litfile, tmpkey);
        if (status) return status;
        status = lit_get_file(litfile, sealed_string, &ptr, &size);
        if (status) return status;

        deskey(tmpkey, DE1);
        for (ofs = 0; ofs < size; ofs += 8)
        {
            des( (ptr+ofs), (ptr+ofs));
        }
        if (*ptr) {
            lit_error(ERR_R,
                "Unable to decrypt title key!  Check value = %02x\n"
                "This may be a newer or corrupted .LIT file!", *ptr);
            litfile->drmlevel = DRM_LEVEL_INVALID;
            free(ptr);
            return E_LIT_DRM_ERROR;
        }
        /* the key is the 8 bytes, skipping the first (should be 0) */
        memcpy(litfile->bookkey, ptr+1, 8);
        free(ptr); 
        ptr = NULL;
    } 
    else {  /* DRM level is 5 */
        if (litfile->drm5_callback) {
            litfile->drmlevel = 5;
            status = litfile->drm5_callback(litfile->drm5_data,NULL,0);
            return status;
        }
        else {
            lit_error(ERR_R, "DRM5 is not supported!");
            litfile->drmlevel = 5;
            return E_LIT_DRM_ERROR; 
        }
    }

    status = lit_get_file(litfile,validation_string,&ptr,&size);
    if (status) {
        litfile->drmlevel = DRM_LEVEL_INVALID;
        return status;
    }
    if (strncmp(msreader_string,ptr,strlen(msreader_string)) != 0) {
        lit_error(ERR_R, 
            "Decryption is incorrect -- the Validation string is wrong!");
        free(ptr);
        litfile->drmlevel = DRM_LEVEL_INVALID;
        return E_LIT_DRM_ERROR;
    }
    free(ptr);
    return 0;
}

/*--[lit_i_encrypt]------------------------------------------------------------
 |
 | This routine encrypts the section with the specified key
 | 
*/
int lit_i_encrypt(U8 * pContent, int sizeContent, U8 * new_key)
{
    int     ofs;

    deskey(new_key, EN0);
    for (ofs = 0; ofs < sizeContent; ofs+= 8)
    {
        des( (pContent+ofs), (pContent+ofs));
    }
    return 0;
}


/*--[lit_i_decrypt]------------------------------------------------------------
 | 
 | This routine handles decryption of sections.  
 |
*/
int lit_i_decrypt(lit_file * litfile, U8 * pContent, int sizeContent)
{
    int     status; 
    int     ofs;

    if (litfile->drmlevel < 0) { 
        return E_LIT_DRM_ERROR;
    }
    if (litfile->drmlevel == 5) {
        status = E_LIT_DRM_ERROR;
        if (litfile->drm5_callback) {
            status = litfile->drm5_callback(litfile->drm5_data, 
                pContent, sizeContent);
        } else {
            lit_error(ERR_R,
                "This version is unable to decrypt owner-exclusive content!");
        }
        return status;
    }
    deskey(litfile->bookkey, DE1);
    for (ofs = 0; ofs < sizeContent; ofs+= 8)
    {
        des( (pContent+ofs), (pContent+ofs));
    }
    return 0;
}

/*--[lit_change_drm_level]-----------------------------------------------------
 |
 | This routine changes the DRM on the file. 
 | Specifically, this can go from 5 or 3 to 1, and from 1 to 3 (inscribing).
 | DRM2 and DRM4 are undefined.
 | and DRM0 is unsupported for now   
*/
int lit_change_drm_level(lit_file * litfile, int newlevel, U8 * drm_data,
    int data_size)
{
    int     i, status;
    U8      * new_sealed, * pBookplate, new_key[8];
    

    if (litfile->drmlevel == 0) {
        lit_error(ERR_W,"Changing DRM0 is not yet supported.");
        return E_LIT_UNSUPPORTED;
    }
    if (newlevel == 0) { 
        lit_error(ERR_W,"Converting TO DRM0 is not yet supported.");
        return E_LIT_UNSUPPORTED;
    }
   
    if (newlevel == 3) {
        if (!drm_data) {
            lit_error(ERR_W,"No inscription data supplied!");
            return E_LIT_NULL_POINTER;
        }
    }
    lit_remove_files(litfile, license_path_string);
    switch (newlevel)
    {
    case 1:
        lit_remove_files(litfile, bookplate_string);
        break;
    case 3: 
        pBookplate = malloc(data_size);
        if (!pBookplate) {
            lit_error(ERR_W,"Can't allocate %d bytes for copy of inscription.",
                data_size);
            return E_LIT_OUT_OF_MEMORY;
        }
        memcpy(pBookplate, drm_data, data_size);
        status = lit_put_file(litfile,bookplate_string,pBookplate,data_size, 0);
        if (status) return status; 
        break;
    default:
        lit_error(ERR_W,"Unable to change DRM level to %d.", newlevel);
        return E_LIT_UNSUPPORTED;
    }
    if ((litfile->drmlevel == 5) || (litfile->drmlevel == 0)) {
        /* Need a new bookkey */
        random_data(new_key, 8);
        for (i = 1; i < litfile->num_sections; i++) {
            status = lit_i_encrypt_section(litfile,litfile->sections[i].name,
                &new_key[0]);
        }        
        memcpy(litfile->bookkey, new_key, 8);
    } 
    new_sealed = malloc(SEALED_SIZE);  
    if (!new_sealed) { 
        lit_error(ERR_W,"Not enough space for new sealed."); 
        return E_LIT_OUT_OF_MEMORY;
    } 
    memset(new_sealed, 0, SEALED_SIZE);
    memcpy(new_sealed+1, litfile->bookkey, 8);

    litfile->drmlevel = newlevel; 

    status = calculate_deskey(litfile,new_key); 
    if (status ) {
        lit_error(ERR_W, "Unable to make new \"Sealed\" key.");
        free(new_sealed);
        return status;
    }
    deskey(new_key, EN0);
    for (i = 0; i < SEALED_SIZE; i+=8) { 
        des( (new_sealed+i), (new_sealed+i)); 
    }
    status = lit_put_file(litfile,sealed_string,new_sealed,
        SEALED_SIZE,0);
    if (status) {
        free(new_sealed);
        return status;
    }

    return 0;
}


/*--[hash_bytes]---------------------------------------------------------------
 |
 | This is a "special" hash routine which has slightly different properties
 | than just calling SHA_Update. 
 | Note that the SHA routine isn't exactly standard either.
*/
void hash_bytes(SHA_CTX * pctx, U8 * pdata, int length, int padding)
{
    /* This buffer size is FIXED as per the algorithm */
    U8      buffer[64];
    int     pos, avail, offset;

    if (padding > sizeof(buffer))
    {
        return;
    }
    memset(buffer, 0, padding);
    pos = padding;
    offset = 0;

    while (length > 0)
    {
        avail = sizeof(buffer) - pos;
        if (length >= avail)
        {
            memcpy(&buffer[pos],pdata+offset, avail);
            length -= avail;
            offset += avail;
        } else {
            /* Length + 0's */
            memcpy(&buffer[pos],pdata+offset, length);
            memset(&buffer[pos+length],0, avail - length);
            /* offset is meaningless now */
            length = 0;
        }    
        SHA1_Update(pctx, buffer, sizeof(buffer));
        pos = 0;
    }
}


/*--[calculate_deskey]---------------------------------------------------------
 |
 | This routine is responsible for calculating a DES key from the internal
 | files. 
 | See the comment above for which files affect the calculation process.
 | Things are done in a particular way for various reasons and small changes
 | will invalidate the key calculation process.
 | 
*/
int calculate_deskey(lit_file * litfile, U8 key[8])
{
    int      nbytes;
    int      status, i, pad;
    U8       *p, digest[20];
    SHA_CTX  ctx;
    const char *hashfiles[] = {meta_string,drmsource_string,
                               bookplate_string,NULL};
    const char **s;

    SHA1_Init(&ctx);
    /* Remove the bookplate if it is not present. */
    if (litfile->drmlevel != 3) {
        hashfiles[2] = 0;
    }

    s = hashfiles;
    /* First hash has two bytes of padding */
    pad = 2;
    while (*s)
    {
        status = lit_get_file(litfile, *s, &p, &nbytes);
        if (status) { return status; }
        hash_bytes(&ctx, p, (int)nbytes, pad);
        free(p);
        pad = 0;
        s++;
    }
    SHA1_Final(digest, &ctx);

    memset(key, 0, 8);
    for (i = 0; i < sizeof(digest); i++)
    {
       key[i%8] ^= digest[i];
    }
    return 0;
}

/*--[random_data]--------------------------------------------------------------
 |
 | Random data for creating book key's.
 | FUTUREFIX - This is very non-random.
*/
void random_data(U8 * pData, int length)
{
    int i, r;

    srand(time(NULL));
    for (i = 0; i < length; i++) {
        r = rand() ^ (rand() >> 7);
        pData[i] = (r & 0xff); 
    } 
}
