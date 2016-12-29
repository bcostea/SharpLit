/*--[drm5.c]-------------------------------------------------------------------
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
/* This file decrypts a DRM5 encrypted title, assuming the private key
 * is provided. This allows Convert Lit to execute much faster. 
 * 
 * Note that the July 4th update added only a trivial amount of complexity
 * to this process. 
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ext/litlib.h"
#include "utils.h"
#include "ext/des/d3des.h"

#include "ext/tom/tommath.h"

static const char * license_string = "/DRMStorage/Licenses/EUL";

const unsigned long int obfuscation_keys[] = {0,0x6E12CF4A,0x36A2732C};

U8 * read_whole_file(char * filename, int * size_ptr)
{
    U8 *mem = NULL;
    size_t size;
    FILE    *f;
	errno_t err;

	err = fopen_s(&f, filename, "rb");
    if (err!=0) {
        lit_error(ERR_LIBC,"WARNING: Unable to open file \"%s\".", filename);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        free(mem);
        fclose(f);
        lit_error(ERR_LIBC,"fseek() failed.");
        return NULL;
    }
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    mem = malloc(size+1);
    if (!mem) {
        fclose(f);
        lit_error(ERR_LIBC,"malloc(%d) failed.", size);
        return NULL;
    }
    if (fread(mem, size, 1, f) != 1) {
        fclose(f);
        free(mem);
        lit_error(ERR_LIBC,"Unable to read %d chars.",
            size);
        return NULL;
    }
    fclose(f);
    mem[size] = '\0';
    if (size_ptr) *size_ptr = size;
    return mem;
}

extern char * key_file;
extern char dir_lit_file[];
extern char dir_program[];
/* 
 | get_next_key
 | returns 1 if there's a private_key/modulus pair available
 |              (and, by implication, there is more)
 | returns 0 if there isn't.
 */
int get_next_key(char * private_key_ptr, char * modulus_ptr)
{
    static int     which_file = 0,got_one_file;
    static U8      * pFile = NULL;
    static int     remaining;
    static U8      * pKey, * pModulus, * p;  
    static char    * sKeysFile = "keys.txt";
    U8     * s;

    do 
    {
        while (!pFile) {
            switch (which_file)
            {
            case 0:
                got_one_file = 0; 
                if (key_file)
                    pFile = read_whole_file(key_file, &remaining);
                break;
            case 1:
                pFile = read_whole_file(sKeysFile, &remaining);
                break;
            case 2:
                pFile = NULL;
                s = strmerge(dir_program, sKeysFile, NULL);
                if (s) pFile = read_whole_file(s, &remaining);
                if (s) free(s);
                break;
            case 3:
                pFile = NULL;
                s = strmerge(dir_lit_file, sKeysFile, NULL);
                if (s) pFile = read_whole_file(s, &remaining);
                if (s) free(s);
                break;
            default:
                if (!got_one_file) {
                    lit_error(0,"No \"keys.txt\" files found. Can't decrypt.");
                }
                return 0; /* No more files */
            }
            which_file++;
            if (pFile) { got_one_file++; }
            pModulus = NULL;
            pKey = p = pFile;
        }
        if (!pFile) return 0;

        while (remaining) {
            if (*p == '\n') {
                *p = '\0';
                if ((!pKey) || (pModulus)) {
                    if (pModulus && (p - pModulus) > 1) {
                        strcpy(private_key_ptr, pKey);
                        strcpy(modulus_ptr, pModulus);
                        pKey = p+1; pModulus = NULL;
                        return 1;
                    }
                    pKey = p+1; pModulus = NULL;
                } else if (!pModulus) {
                    if ((p - pKey) > 1)
                        pModulus = p+1;
                    else 
                        pKey = p+1;
                }
            }
            if ((*p == '\r') || (*p == '\n') || (*p == ' ') || (*p == '\t') ||
                (!*p)) {
                *p = 0;
            }
            else if ( (*p < '0') || (*p > '9')) {
                pKey = pModulus = NULL;
            }
            p++;
            remaining--;
        }
        if (pKey && pModulus) {
            strcpy(modulus_ptr, pModulus);
            strcpy(private_key_ptr, pKey);
            pModulus = pKey = NULL; 
            /* Next time, we come through, and skip this fork */
            return 1; 
        }
        free(pFile); pFile = p = NULL;
    } while (1);
    return 0;
}

int ms_base64_decode(char * str, unsigned char *buff, int bufflen)
{
    char *cp;
    char *ocp;
    int val, count, block, ocount;

    ocp = buff;
    count = 0;
    block = 0;
    ocount = 0;
    for (cp = str; *cp != '\0'; cp++) {
        if (ocount >= bufflen)
        {
            return -1;
        }
        if ((*cp >= 'A') && (*cp <= 'Z'))
            val = *cp - 'A';
        else if ((*cp >= 'a') && (*cp <= 'z'))
            val = *cp - 'a' + 26;
        else if ((*cp >= '0') && (*cp <= '9'))
            val = *cp - '0' + 52;
        else if ((*cp == '+') || (*cp == '!'))
            val = 62;
        else if ((*cp == '/') || (*cp == '*'))
            val = 63;
        else if (*cp == '=') {
            if (count == 2) {
                *ocp++ = block >> 4;
                ocount++;
            } else {
                *ocp++ = (block >> 10);
                *ocp++ = (block >> 2) & 0xff;
                ocount += 2;
            }
            break;
        } else
            val = 1;

        if (val >= 0) {
            block = (block << 6) | val;
            if (++count == 4) {
                *ocp++ = block >> 16;
                *ocp++ = (block >> 8) & 0xff;
                *ocp++ = block & 0xff;
                ocount += 3;
                count = 0;
            }
        }
    }
    return ocount;
}

/* Stupid little fake XML parser. */
static char *find_close(char * str)
{
    while ((*str != '\0') && (*str != '>')) {
        if (*str == '"') {
            if ((str = strchr(str + 1, '"')) == NULL)
                return NULL;
        }
        str++;
    }

    if (*str == '\0')
        return NULL;
    else
        return str + 1;
}


char *get_element(char * tag, char * str)
{
    int len = strlen(tag);
    char *tmptag;
    char *start, *end;
    char *rval = NULL;
    char *tmp;

    if ((tmptag = malloc((len + 4) * sizeof(char))) == NULL) {
        lit_error(ERR_R, "Memory allocation failed in get_element()"); 
        return NULL;
    }
    strcpy_s(tmptag, strlen(tmptag) + 1, "<");
    strcat_s(tmptag, len + 4,  tag);

    while (1) {
        if ((start = strstr(str, tmptag)) == NULL)
            goto exit;
        if (!isalnum(start[len + 1]))
            break;
        str = start + len + 1;
    }
    strcpy_s(tmptag, strlen(tmptag) + 3, "</");
    strcat_s(tmptag, strlen(tmptag) + strlen(tag), tag);
    strcat_s(tmptag, strlen(tmptag) + 1, ">");
    end = strstr(str, tmptag);
    if (end == NULL) {
        goto exit;
    } else {
        char *realstart = find_close(start);
        if ((realstart == NULL) || (realstart > end)) 
            goto exit;
        tmp = malloc((end - realstart + 1) * sizeof(char));
        if (tmp == NULL) {
            lit_error(ERR_R, "memory allocation #2 failed in get_element()"); 
            return NULL;
        }
        memcpy(tmp, realstart, (end - realstart) * sizeof(char));
        tmp[end - realstart] = '\0';
        rval = tmp;
    }
exit:
    free(tmptag);
    return rval;
}

int my_mp_to_unsigned_bin(mp_int * a, unsigned char * b, int len)
{
    int x, res;
    mp_int t;

    memset(b, 0, len);
    if ((res = mp_init_copy(&t, a)) != MP_OKAY) { return res; }
    x = 0;
    while ((mp_iszero(&t) == 0) && (x < len)) {
        b[x++] = (unsigned char)(t.dp[0] & 255);

        if ((res = mp_div_2d(&t, 8, &t, NULL)) != MP_OKAY) {
            lit_error(0,"MP_DIV_2D FAILED!  %d\n", res);
            mp_clear(&t); return res;
        }
    }
    mp_clear(&t);
    return 0;
}


void little2big(unsigned char * p, int len)
{
    int i;

    len--;
    for (i = 0; i < (len/2)+1; i++) {
        if (i != (len -i)) {
            p[i] ^= p[len - i];
            p[len - i] ^= p[i];
            p[i] ^= p[len - i];
        }
    }
}

unsigned char drm5_des_key[16]; 
char sModulus[512/3]; 
char sPrivateKey[512/3];
unsigned char keyData[256];

int drm5_handle_key(lit_file * plf)
{
    U8  * pData;
    int size, r, i, j;
    char  *ebits = NULL, * value = NULL;
    int keySize, status;
    U8 * pValidation;
    mp_int  c, d, p, mod;
    char    des_key[64];


    status = lit_get_file(plf,license_string,&pData,&size);
    if (status) {
        lit_error(ERR_R, 
"Was unable to find an End-User-License (\"%s\") in the LIT file.", 
	license_string); 
        return status;
    }

    {
        for (i = 0; i < (size/2); i++) pData[i] = pData[i*2];
        pData[i] = '\0';
        size /= 2;
    } 
    pValidation = NULL;

    ebits = get_element("ENABLINGBITS", (char *)pData);
    if (ebits) value = get_element("VALUE", ebits);
    if (!value) {
        lit_error(ERR_R, 
"I couldn't find an encrypted key in the End User License file.");
        free(pData);
        return -1;
    }
    keySize = ms_base64_decode(value, keyData, sizeof(keyData));
    little2big(keyData, keySize);
    r = E_LIT_DRM_ERROR;
    while (get_next_key(sPrivateKey, sModulus))
    {
        int res;

        res = mp_init(&c);
        if (!res) res = mp_init(&p);
        if (!res) res = mp_init(&mod);
        if (!res) res = mp_init(&d);
        if (!res) res = mp_read_radix(&p, sPrivateKey, 10);
        if (!res) res = mp_read_radix(&mod, sModulus, 10);
        if (!res) res = mp_read_unsigned_bin(&c, keyData, keySize);
        if (!res) res = mp_exptmod(&c,&p,&mod, &d);
        if (!res) res = my_mp_to_unsigned_bin(&d, des_key, sizeof(des_key));
        if (res) {
            lit_error(0,"MP Library error doing RSA decryption = %d.\n",res);
            return -1;
        }
        for (j = 8; j < 62; j++) 
            if (des_key[j]) { break; }
        if (j != 62) {
            continue;
        }
        
        r = -1;
        for (j = 0; j < sizeof(obfuscation_keys)/sizeof(obfuscation_keys[0]);
            j++) {
            U32   x;

            memcpy(drm5_des_key, des_key, 8);
            x = obfuscation_keys[j]*2; 
            for (i = 2; i < 6; i++) {
                drm5_des_key[i] ^= (x&0xff);
                x >>= 8;
            }
            x = obfuscation_keys[j];
            for (i = 4; i < 8; i++) {
                drm5_des_key[i] ^= (x&0xff);
                x >>= 8;
            }
            x = obfuscation_keys[j]; 
            if (x) {
                x = (~x) >> 1;
                for (i = 0; i < 4; i++) {
                    drm5_des_key[i] ^= (x&0xff);
                    x >>= 8;
                }
            }
            status = lit_get_file(plf,"/DRMStorage/ValidationStream",
                &pValidation, &size);
            if ((status) || (!pValidation) || (!size)) {
                lit_error(ERR_R,"Unable to read validation stream.");
                free(pData);
                return -1;
            }
            if (strncmp(pValidation,"MSReader",8) != 0) {
                /* Cleanse the bad sections */
                for (i = 0; i < plf->num_sections; i++)
                {
                    if ((plf->sections[i].data_pointer) && 
                        (plf->sections[i].data_pointer != (U8 *)-1))
                    {
                        free(plf->sections[i].data_pointer);
                        plf->sections[i].data_pointer = NULL;
                    }
                }
                free(pValidation); pValidation = NULL;
            } else { r = 0; break; }
        }
        if (!r) break;
    }
#if 0
    if (r != 0) {
        lit_error(0,"Failed to decrypt this title!");
    }
#endif
    free(pData);
    if (pValidation) free(pValidation);
    return r;
}

int drm5_decrypt(U8 *ptr, int size)
{
    int ofs;

    deskey(drm5_des_key, DE1);
    for (ofs = 0; ofs < size; ofs += 8) { des( (ptr+ofs), (ptr+ofs)); }
    return 0;
}


/*--[drm5_callback]------------------------------------------------------------
 |
 | This routine will handle interfacing to the main lit library.  
 |
 | If p is NULL, then initialize..
 | Otherwise, decrypt and hand back the data.
 | 
*/
int drm5_callback(void * pv, U8 * p, int size)
{
    int status = 0;

    if (!p) {
        if (!status) status = drm5_handle_key((lit_file *)pv);
    }
    else { status =  drm5_decrypt(p, size); };
    return status;
}
