/*--[explode.c]----------------------------------------------------------------
 | Copyright (C) 2002, 2003 Dan A. Jackson
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "clit.h"
#include "ext/litlib.h"
#include "manifest.h"
#include "utils.h"
#include "explode.h"

#ifdef _MSC_VER
#include <direct.h>
#define mkdir(x,y) _mkdir(x)
#define strncasecmp _strnicmp
#endif

/*
 | Herein is the routines to handle "exploding" of .LIT files into their
 | component pieces.  
 | Some features, most noteably parsing the "/manifest" and expanding the
 | tag-compressed HTML are included in the library.
 */

extern int disable_directory_support;

manifest_type   manifest;
extern char *   writingFilename;

/* Leave off the newline intentionally */
const char * meta_string = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\
<!DOCTYPE package \n\
  PUBLIC \"+//ISBN 0-9673008-1-9//DTD OEB 1.0.1 Package//EN\"\n\
  \"http://openebook.org/dtds/oeb-1.0.1/oebpkg101.dtd\">";
const char * xhtml_string = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\
<!DOCTYPE html PUBLIC \n\
 \"+//ISBN 0-9673008-1-9//DTD OEB 1.0.1 Document//EN\"\n\
 \"http://openebook.org/dtds/oeb-1.0.1/oebdoc101.dtd\">";

int  directory_flag = 0;
int  create_placeholders(char * pathOutput,manifest_type * pManifest);
char * get_opf_name(char * pathOutput, char * litname);
int  write_htmlish_file(lit_file * litfile, char *, char *, char *);
int  write_raw_file(lit_file * litfile,char *pathInternal, char *pathExternal);

int  create_full_path(char * pathStarting, char * pathFile);

int explode_lit(lit_file * litfile, char * litName, char * pathOutput) 
{
    char            * pathOPF = NULL;
    char            * pathInternal = NULL;
    char            * path = NULL;
    int             status;
    int             stage, i;

    status = lit_read_manifest(litfile, &manifest);
    if (status) goto bad;

    status = create_placeholders(pathOutput,&manifest);
    if (status) goto bad;

    pathOPF = get_opf_name(pathOutput,litName); 
    writingFilename = pathOPF;
    status = write_htmlish_file(litfile, "/meta", pathOPF, NULL);
    free(pathOPF); pathOPF = NULL;
    writingFilename = NULL; 
    if (status) goto bad;

    for (stage = 0; stage < NUM_MANIFEST_MAPS; stage++) {
        name_mapping * map;

        if (!manifest.mappings[stage]) continue;
        map = manifest.mappings[stage];
        for (i = 0; i < manifest.num_mappings[stage]; i++) {
            path = strmerge(pathOutput,(char *)map->sOriginal,NULL);
            if (!path) {
                status = E_LIT_OUT_OF_MEMORY;
                goto bad;
            }
            writingFilename = path;
            show_information("Writing out \"%s\" as \"%s\" ...",
                map->sInternal, map->sOriginal);

            status = E_LIT_INTERNAL_ERROR;
            switch (stage) {
                case 0:
                case 1:
                    pathInternal = NULL; /* Generated inside htmlish code */
                    status = write_htmlish_file(litfile,(char *)map->sInternal,
                        path,(char *)map->sOriginal);
                    break; 
                case 2:
                case 3:
                    pathInternal = strmerge("/data/",(char *)map->sInternal,0);
                    if (!pathInternal) {
                        status = E_LIT_OUT_OF_MEMORY;
                        goto bad;
                    }
                    status = write_raw_file(litfile,pathInternal,path);
                    break;
            }
            if (status) {
                if (status > 0) {
                     show_information("Wrote out \"%s\", but with errors.\n",
                         path);
                }
                if (status < 0) return -1;
            }
            else {
                show_information("Successfully written to \"%s\".\n", path);
            }
            if (path) { free(path); path = NULL;}
            if (pathInternal) { free(pathInternal); pathInternal = NULL; }
            writingFilename = NULL;
            map++;
 
        }
    }
    
    return  0;  
bad:
    if (path)         free(path);
    if (pathOPF)      free(pathOPF);
    if (pathInternal) free(pathInternal);
    return status;
}

/*
 | Return the length (in "points" not characters!) that is common between all
 | entries.
 */
int find_common_length(manifest_type * pManifest)
{
    mapping_state  state;
    name_mapping   * map;
    U8             * sFirst;
    int            common_length;

    mapping_init(&state, pManifest);
    /**
     * First, find the maximum number of characters that is common between
     * _all_ original filenames.  This will be 0 except for some rare cases.
     */
    map = mapping_next(&state);
    sFirst = NULL;
    common_length = 0;
    /* Common length is in UTF8 units */
    if (map) {
        sFirst = map->sOriginal;
        common_length = utf8_strlen(sFirst);
    }
    while ((map = mapping_next(&state)) != NULL)
    {
        int new_common;
        new_common = utf8_strmatch(sFirst, map->sOriginal);
        if (new_common < common_length) common_length = new_common;
    }
    return common_length;
}

/*
 | Create a single file from a name mapping,
 | Creates directories as needed. 
 */
U8 * create_file_path(U8 * pathOutput, U8 * pathFile)
{
    U32         c;
    int         error, len, status;
    utf8_iter   iter;
    U8          * sFiltered, * sDirname, * sTemp, * s, * p;
    FILE        * f;
    
    sFiltered = sTemp = sDirname = NULL;
    do {
        sFiltered = strmerge(pathFile, NULL,NULL);
        if (!sFiltered) return NULL;    
        sDirname = strmerge(pathFile, NULL,NULL);
        if (!sDirname) return NULL;    

        error = 0;
        /* Loop one..
         *  Filter 
         */
        utf8_start(&iter, pathFile);
        s = sFiltered;
        do { 
            c = utf8_next(&iter);
            if (!c) {
                *s = '\0';
            }
            /* Filter out things which wouldn't be in DOS filenames */
            else if ((c <  ' ')|| (c == '?')|| (c == '<')|| (c == '>')||
                (c == '\"')|| (c == '*')|| (c == '|')|| (c == ';')||
                (c ==0x7f) || (c == ':'))
            {
                /* By definition, this can't be UTF8! */
                *(s++) = '_';
            } else if ((c == '/') || (c == '\\')) {
                /* Use common slash, This is differentated later */
                *(s++) = '/';
            } else {
                len = utf8_store(s, &iter);
                s += len;
            }
        } while (c);

        s = sDirname;
        utf8_start(&iter, sFiltered);
        do {
            p = utf8_next_token(&iter, '/', &len);
            if (!len) break;

            /* Skip dots.  */
            if ((utf8_strmatch(p,"..") == 2) || 
                (utf8_strmatch(p,".") == 1)) {
                continue;
            }
            memcpy(s, p, len);
            s += len;
            *s = '\0';

            /* This isn't a directory, continue on to the file check */
            if (utf8_peek(&iter) == 0) break; 

            sTemp = strmerge(pathOutput,sDirname, NULL);
            if (!sTemp) { error = 9; break; }

            directory_flag++;

            if (disable_directory_support) {
                /* Shouldn't be here, because the other check takes 
                 * care of it.  Just in case.... */
                error = 11;
                break;
            }

            status = mkdir(sTemp,0755);
            if (status && (errno != EEXIST)) {
                lit_error(ERR_LIBC,"WARNING: mkdir() failed for \"%s\".\n",
                    sTemp);
                free(sTemp); sTemp = NULL;
                error = 10;
                break;
            }
            free(sTemp); 
            sTemp = NULL;

#ifdef _MSC_VER
            *(s++) = '\\';
#else
            *(s++) = '/';
#endif
        } while (1);

        free(sFiltered);
        sFiltered = NULL;

        /* If an error occurred, go on to the next (pathless) potential name */
        if (error) break;
        
        /* Str should now contain a full qualified path, try to make the
         * file. */
        f = NULL;
            
        sTemp = strmerge(pathOutput, sDirname, NULL);
        if (sTemp) {
			errno_t err = 0;

			err = fopen_s(&f, sTemp, "r");
            if (err==0) { error = 1; }
            else {
                err = fopen_s(&f, sTemp,"w");
                if (err==0) { fclose(f); error = 0; } 
                else { error = 2; }
            }
        } else { error = -1; break; }
    } while (0);

    if (sTemp) free(sTemp);
    if (sFiltered) free(sFiltered);
    if (error) { if (sDirname) free(sDirname); sDirname =  NULL; } 
    return sDirname;
}

char * int2string(int num)
{
    static char sNum[10];
    char *sNumPtr;
    int temp = abs(num);
    
    sNumPtr = &sNum[sizeof(sNum)/sizeof(sNum[0]) - 1];
    *(sNumPtr--) = '\0';
    do {
        *sNumPtr = '0' + (temp%10);
        sNumPtr--;
        temp /= 10;
    } while (temp && (sNumPtr > sNum));
    if (num < 0)
        *sNumPtr = '-';
    else 
        sNumPtr++;
    return sNumPtr;
}

/*
 | This routine is responsible for converting from the "original" names
 | stored in the .LIT file into names on the local file system.
 | 
 | To fix duplicates, this CREATES all the files as it goes along.  
 | It also permits recreating a directory tree.
 |
 | Steps:
 | 1. Filter out the common portions.  
 |    (Workaround for a buggy generator which stores the originals as:
 |      C:\Something\Else\Books\ISBN\index.html).
 |
 | 2. Starting at the unique portion, create the subdirectories
 |    Subdirectories aren't handled at the moment, due to relative pathing.
 |    Filter out backwards references.
 | 
 | 3. Finally, create the file.  if it exists, try some numbers.
 | 
 | 4. If all the numbers are taken, start over with the Internal filename
 |
 */
int  create_placeholders(char * pathOutput,manifest_type * pManifest)
{
    int units2skip;
    mapping_state  state;
    name_mapping   * map;

    units2skip = find_common_length(pManifest);

    /* For each entry, find the unique portion and start the games */
    mapping_init(&state, pManifest);
    while ((map = mapping_next(&state)) != NULL) {
        int bytes, units, len, bytes_after_dot;
        U8  *p, * slash, * dot;
    	U32 c;

        /* Skip to the last slash */
        if (disable_directory_support)
            units2skip = 0x7FFFF;

        slash = p = map->sOriginal;
    	bytes = strlen(p)+1;
    	units = 0;
    	while (units < units2skip) {
            len = read_utf8_char(p, bytes, &c);
            if ((!c) || (len <= 0)) break;
            if ((c == '/') || (c == '\\')) slash = (p+len);
            units++;
            p += len;
        }
        p = dot = slash;  
        while (1) {
            len = read_utf8_char(p, bytes, &c);
            if ((!c) || (len <= 0)) break;
            if (c == '.') dot = p;
            units++;
            p += len;
        }
        bytes_after_dot = p - dot;
        if (!bytes_after_dot) { dot = NULL; }

        /* Start at the previous slash or the beginning */
        p = create_file_path(pathOutput, slash);
        if (!p) {
            U8 * s;
            int     count;

            s = strmerge(map->sInternal, dot, NULL);
            if (s)
                p = create_file_path(pathOutput, s);
            if (!p)
            {
                if (s) free(s);
                for (count = 1; count < 1000; count++)
                {
                    s = strmerge(map->sInternal, int2string(-count), dot);
                    if (s) {
                        p = create_file_path(pathOutput,s);
                    }
                    if (p) break;
                }
                if (count == 1000) {
                    lit_error(0, "Unable to create a DOS file for \"%s\". \n",
                        map->sInternal);
                    return -1;
                }
            }
        }
        if (p) {
            free(map->sOriginal);
            map->sOriginal = p;
        }
    }
    return 0;
}        

char * get_opf_name(char * pathOutput, char * litname)
{
    int idx, len;
    char * str;

    idx = len = strlen(litname);

    /* assume that litname is a valid LIT file */
    while (idx) {
        if ((litname[idx] == '/') ||
            (litname[idx] == '\\') || 
            (litname[idx] == ':'))  {
            idx++;
            break;
        }
        idx--;
    }
    if (strncasecmp(&litname[len-4],".lit",4) == 0) {
        str = strmerge(pathOutput,&litname[idx],NULL);
        if (!str) return str;
        strcpy_s(&str[strlen(str)-3], 3, "opf");
        return str;
    } 
    return strmerge(pathOutput,&litname[idx],".opf");
}

int write_callback(void * v, U8 * pData, int nBytes)
{
    FILE    * f;
    int     r;

    if (!v) { 
        f = stdout; 
    } else {
        f = (FILE *)v;
    }
    r = fwrite(pData,1, nBytes,f);
    if (r < 0) {
        lit_error(ERR_W|ERR_LIBC,"fwrite of %d bytes failed!",nBytes);
    }
    return r;
}


int  write_htmlish_file(lit_file * litfile, char * source_name,
    char * pathExternal, char * pathOriginal)
{
    U8  * p;
    lit_atom_list * listAtoms;
    int nbytes;
    int status;
    FILE * fOut;
    char * pathInternal;
    manifest_type * pmanifest, relative_manifest;

    pmanifest = &manifest;
    p = NULL;
    fOut = NULL;
    listAtoms = NULL;
    pathInternal = NULL;

    status = -1;
    do {
        if (pathOriginal != NULL)
        {
            pathInternal = strmerge("/data/",source_name, "/content");
        }
        else
        {   /* the meta files, don't have the 3-piece string */
            /* So, copy anyway - it only happens once */
            pathInternal = strmerge(source_name, 0, 0 );
        }
        if (!pathInternal) { status = E_LIT_OUT_OF_MEMORY; break; }
        if (pathOriginal)
        { 
            listAtoms = lit_read_atoms(litfile, source_name);
        }

        status = lit_get_file(litfile,pathInternal,&p, &nbytes);
        if (status) { break; }

        if ((directory_flag) && pathOriginal && !disable_directory_support) {
            status = make_relative_manifest(pathOriginal,&relative_manifest, 
                pmanifest);
            if (status != 0) { status = -20; break;}
            pmanifest = &relative_manifest;
        }
		errno_t err;

		err = fopen_s(&fOut, pathExternal, "w");
        if (err != 0) {
            lit_error(ERR_LIBC|ERR_W,"fopen(%s) failed!", pathExternal);
            status =  -1;
            break;
        }
        if (pathOriginal == NULL) {
            status = write_callback((void *)fOut,(U8 *)meta_string, 
                  strlen(meta_string));
            if ((size_t)status != strlen(meta_string)) { status = -1; break; }
            status = lit_reconstitute_html(p, 0, nbytes, 1, pmanifest, 
                listAtoms, write_callback, (void *)fOut);
        }
        else {
            status = write_callback((void *)fOut,(U8 *)xhtml_string, 
                  strlen(xhtml_string));
            if ((size_t)status != strlen(xhtml_string)) { status = -1; break;}
            status = lit_reconstitute_html(p, 0, nbytes, 0, pmanifest, 
                listAtoms, write_callback, (void *)fOut);
        }
        if ((status > 0) && (status != nbytes)) {
            lit_error(ERR_R,
"Warning - Couldn't fully expand \"%s\", decoded %d out of %d.\n",
                pathInternal, status, (int)nbytes);
            status = 1;
        }
        else if (status < 0) {
            lit_error(ERR_R,
"ERROR - Failure during tag expansion of \"%s\" Code: %d!\n", pathInternal, 
            status);
        }
        else status = 0;

    } while (0);
    if (fOut != NULL) fclose(fOut);
    if (p) free(p);
    if (pathInternal) free(pathInternal);
    if (listAtoms) { lit_free_atoms(listAtoms); }
    return status;
}

int write_raw_file(lit_file * litfile,char * pathInternal, char * pathExternal)
{
    int     status;
    int     nbytes;
    U8      * p;
    FILE * fOut;
	errno_t err;

    status = lit_get_file(litfile,pathInternal,&p,&nbytes);
    if (status) return status;

    err = fopen_s(&fOut, pathExternal, "wb");
    if (err!=0) {
        free(p);
        lit_error(ERR_LIBC|ERR_W,"fopen(%s) failed!", pathExternal);
        return -1;
    }
    status = fwrite(p,1, nbytes,fOut);
    free(p); p = NULL;

    if (status < 0) {
        lit_error(ERR_W|ERR_LIBC,"Failed to write %d bytes.",nbytes);
        return status;
    }
    if (status != nbytes) {
        lit_error(ERR_W,"Partial write, wrote %d out of %d.",status,nbytes);
        return -1;
    }
    fclose(fOut);
    return 0;
}
