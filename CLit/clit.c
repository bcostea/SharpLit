/*--[clit18.c]----------------------------------------------------------------
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

#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h>
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "ext/litlib.h"


extern int drm5_callback(void *, U8 *, int size);
#ifdef _MSC_VER
#include <direct.h>
#define mkdir(x,y) _mkdir(x)
#define stat  _stat
#endif

#ifndef MAX_PATH
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

/*extern int display_lit(lit_file * litfile, int public_only);
extern int explode_lit(lit_file * litfile, char * litName, char * pathOutput);
extern int transmute_lit(lit_file * litfile, char * newlitfile, char * );
*/
static U8 * readat(void * v, U32 offset, int size);
/* Must not be static, linked into litlib */
void lit_error(int what, char * fmt, ...); 

static int  windowed_mode;
static int  printed_error = 0;
char * readingFilename = NULL;
char * writingFilename = NULL;
int  disable_directory_support = 0;
char   dir_program[MAX_PATH];
char   dir_lit_file[MAX_PATH];
char * key_file = NULL;

const char * sTitle = 
"+---[ ConvertLIT (Version 1.8) ]---------------"\
"[ Copyright (c) 2002,2003 ]---\n"\
"ConvertLIT comes with ABSOLUTELY NO WARRANTY; for details\n"\
"see the COPYING file or visit \"http://www.gnu.org/license/gpl.html\". \n"\
"This is free software, and you are welcome to redistribute it under \n"\
"certain conditions.  See the GPL license for details.\n";

const char * sUsage = \
"\nThis program has three modes of operation: \n"\
"First, is ** EXPLOSION **, or the expanding of a .LIT file into an \n"\
"OEBPS compliant package.  \n"\
"To explode, you type: clit <flags> <litfile.lit> <directory to explode into>\\\n"\
"For Example:\n"\
"\tclit ebook-propietary.lit ebook-oebps\\   \n"\
"If the directory does not exist, you MUST put a trailing \\ or / after it!\n"\
"\nTo disable creating multiple subdirectories, use the \"-d\" flag.\n"\
"\nSecond, is the DOWNCONVERTING of a .LIT file down to \"Sealed\", \n"\
"or DRM1 format for reading on handheld devices. \n"\
"To downconvert, you type:  clit <flags> <litfile.lit> <newlitfile.lit> \n"\
"For Example:\n"\
"\tclit \"drm5 ebook.lit\" ebook-open.lit\n"\
"\nThird, is the INSCRIBING of a .LIT file which allows you to label \n"\
"your ebooks. This is very similar to downconverting, you just add \n"\
"a third argument: clit <flags> <litfile.lit> <newlitfile.lit> <inscription> \n"\
"For example: \n"\
"\tclit ebook.lit inscribed.lit \"the Library of Basil "\
"Frankweiler\"\n"\
"\nDRM5 is supported if you have a \"keys.txt\" file that contains\n"\
"the private key(s) for your passport(s) in either the CLIT program\n"\
"directory or the current directory. \n"\
"Use the -k<keyfile> flag to force the location of your keys.txt\n\n"\
"This is a tool for **YOUR OWN FAIR USE** and not for stealing \n"\
"other people's ebooks.\n"\
"\nPlease do not use this program to distrbute illegal copies of ebooks.\n"\
"\t... that would make Baby Jesus sad.\n";

int main(int argc, char ** argv)
{
    int         status;
    char        * path = NULL;
    lit_file    lit;
    char        * filename, * output, * inscription, c;
    FILE        * fh;
    struct stat statbuf;
    int         task = 0;
    int		    base, i;
    int         done_args = 0;
	errno_t err;

    /* Save the current directory for later */
    strncpy_s(dir_program, MAX_PATH, argv[0], MAX_PATH-1);
    for (i = strlen(dir_program); i >= 0; i--) {
        if ((dir_program[i] == '/') || (dir_program[i] == '\\'))  {
            dir_program[i+1] = '\0'; break;
        }
    }
    printf(sTitle);
    if (argc < 3) { 
        printf(sUsage);
        return -1;
    }
    base = 1;
    while ((argv[base][0] == '-') && (!done_args)) {
        int i;

        for (i = 1; i < (int)strlen(argv[base]); i++) {
            switch (argv[base][i]) {
            case 'd':
            case 'D':
                disable_directory_support = 1;
                break;
            case 'k':
            case 'K':
                key_file = &argv[base][i+1];
                i = 0;
                break;
            case '-':
                done_args = 1;
            }
            if (!i) break;
        }
        base++;
    }
    base--;

    inscription = NULL;
    if (argc == (4+base)) {
        inscription = argv[base+3];
    }
    filename = argv[base+1];

    strncpy_s(dir_lit_file, MAX_PATH, filename, MAX_PATH-1);
    for (i = strlen(dir_lit_file); i >= 0; i--) {
        if ((dir_lit_file[i] == '/') || (dir_lit_file[i] == '\\'))  {
            dir_lit_file[i+1] = '\0'; break;
        }
    }

    output = argv[base+2];
    readingFilename = filename;

    err = fopen_s(&fh, filename, "rb");
    if (err!=0) {
        lit_error(ERR_LIBC|ERR_R,"Unable to open file.");
        return -1;
    }
    memset(&lit, 0, sizeof(lit));
    lit.file_pointer = (void *)fh;
    lit.readat       = readat;
    lit.drm5_callback= drm5_callback;
    lit.drm5_data    = (void *)&lit;
    (void)fseek(fh, 0, SEEK_END);
    lit.filesize = ftell(fh);

    status = lit_read_from_file(&lit);
    if ((status) && (status != E_LIT_DRM_ERROR))
    {
        lit_close(&lit);
        exit(-1);
    }
    printf("LIT INFORMATION.........\n");
    if (lit.drmlevel >= 0) 
        printf("DRM         =  %d    \n", lit.drmlevel);
    printf("Timestamp   =  %08lx \n", lit.timestamp);
    printf("Creator     =  %08lx \n", lit.creator_id);
    printf("Language    =  %08lx \n", lit.language_id);


    if (strlen(output) == 0) {
        if (status) display_lit(&lit, 1);
        else display_lit(&lit, 0);
        lit_close(&lit);
        exit(-1);
    }
    else if (status)
    {
        printf("\nDECRYPTION FAILED - No keys available for this title.\n");
        lit_close(&lit); 
        exit(-1);
    }

    status = stat(output, &statbuf);
  
    if (status < 0) {
        if (errno == ENOENT) { 
            c = output[strlen(output) - 1];
            if ((c == '/') || (c == '\\')) {
                status = mkdir(output,0755);
                if (!status) { 
                    task = 1;
                } else { 
                    lit_error(ERR_LIBC,
"Unable to create directory \"%s\"!", output);
                }
            } else {
                task = 2; 
            }
        } else if (errno == ENOTDIR) {
            fprintf(stderr,"Cannot make directory \"%s\" -- "
                " a file already exists!\n", output);
            exit(-1);
        }else {
            lit_error(ERR_LIBC,"stat() failed unexpectedly");
            exit(-1);
        }
    }
    else {
        if (statbuf.st_mode & S_IFDIR) {
            c = output[strlen(output) - 1];
            if ( (c != '/') && (c != '\\')) {
                path = malloc(strlen(output) + 1);
                if (!path) {
                    fprintf(stderr,"Malloc(%d) failed!\n", strlen(output) + 1);
                    exit(-1);
                }
                strcpy_s(path, strlen(output), output);
				strcat_s(path, strlen(path) + 1,  "/");
                output = path;
            }
            task = 1;
        }
        else {
            status = -1;
            fprintf(stderr,"Output file \"%s\" already exists. "
                "Delete it first!\n", output);
            task = 0;
        }
    }
    if (task == 1) {
        status = explode_lit(&lit, filename, output);            
        if (status != 0) {
            if (status > 0) {
                printf("Finished exploding \"%s\", but with errors.",
                    filename);
            }
        }    
        else {
            printf("Exploded \"%s\" into \"%s\".\n", filename, output);
        }
    } else if (task == 2) {
        status = transmute_lit(&lit, output, inscription);
        if (status != 0) {
            printf("<><><><><> FAILED TO %s \"%s\"!",
                (inscription)?"INSCRIBE":"CONVERT",filename);
        } else {
            printf("Converted \"%s\" --> \"%s\".\n", filename, output);
        }
    }     
    if (task && status)
    {
        if (!printed_error) {
            lit_error(ERR_R,
"Encountered an error, but didn't print a message! Bad programmer!");
        }
    }
    lit_close(&lit);
    if (path) free(path);
    return status;
}

U8 * readat(void * v, U32 offset, int size)
{
    U8 *mem;
    int  read_size;
    FILE * f;

    f = (FILE *)v;
    if (!f) {
        lit_error(0,"No filehandle passed in!");
        return NULL;
    }

    mem = malloc(size);
    if (!mem) {
        lit_error(ERR_LIBC,"malloc(%d) failed.", size);
        return NULL;
    }
    memset(mem, 0, size);
    if (fseek(f, offset, SEEK_SET) != 0) {
        free(mem);
        lit_error(ERR_LIBC|ERR_R,"fseek() failed.");
        return NULL;
    }

    read_size = fread(mem, 1, size, f);
    if (read_size != size) {
        if (feof(f))  {
		fprintf(stderr,
"\nWARNING: Read went past the end of file by %d bytes. \n"\
"If the conversion process doesn't work, try to redownload the file.\n",
			size - read_size);
		return mem;
	}
        free(mem);
        lit_error(ERR_LIBC|ERR_R,"Unable to read %d chars at position %ld.",
            size, offset);
        return NULL;
    }
    return mem;
}

void show_information(char * fmt, ...) {
    va_list     ap;

    if (!windowed_mode) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        printf("\n");
    } else {
#ifdef _MSC_VER

#endif
    }
}

#ifdef _MSC_VER
void DisplayErrorText( DWORD dwLastError ) {
        LPSTR MessageBuffer;
        DWORD dwBufferLength;
        DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS |
                FORMAT_MESSAGE_FROM_SYSTEM;
        if(dwBufferLength = FormatMessageA(dwFormatFlags, NULL,
            dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &MessageBuffer, 0, NULL)) {
            DWORD dwBytesWritten;

            WriteFile( GetStdHandle(STD_ERROR_HANDLE), MessageBuffer,
                dwBufferLength, &dwBytesWritten, NULL);
            LocalFree(MessageBuffer);
        }
}
#endif


void lit_error(int what, char * fmt, ...) {
    va_list     ap;
    unsigned long int       w32err;

    va_start(ap, fmt);
    printed_error++;

    w32err = 0;
#ifdef _MSC_VER
    w32err = GetLastError();
#endif

    if (!windowed_mode) {
        if (((what & ERR_RW) == ERR_R) && readingFilename)
            fprintf(stderr,"Error reading file \"%s\" -- \n\t",
                readingFilename);
        if (((what & ERR_RW) == ERR_W) && writingFilename)
            fprintf(stderr,"Error writing file \"%s\" -- \n\t",
                writingFilename);
        vfprintf(stderr,fmt, ap);
        printf("\n");

        if (what & ERR_LIBC)
            perror("\tLIBC reports");
#ifdef _MSC_VER
        if (what & ERR_WIN32)
            DisplayErrorText(w32err);
#endif

} else {
#ifdef _MSC_VER

#endif
    }
}


