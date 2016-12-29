/*--[litembiggen.c]------------------------------------------------------------
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
/* This file concerns the transformation of "evaporated" html files into 
 | close to their original format.  */ 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include "litlib.h"
#include "littags.h"

/*
 | Warning - Fixed sized buffers here-in. Don't change U32's to U64 without
 | being very sure what you are doing. */ 

char *lit_lookup_mapping(manifest_type * pmanifest,U8 * s,int size);
static int  read_utf8_char(U8 * pdata, int nBytes, U32 * pvalue); 
static int  write_entity(U32 c, HTMLWRITEFCN htmlwrite, void * write_data);

typedef struct entity_map {
    char * name;
    unsigned long int id;
} entity_map;

entity_map entities[] = {{"amp",38},
    {"lt",60}, {"gt",62}};


#define STATE_TEXT                  0
#define STATE_GET_FLAGS             1
#define STATE_GET_TAG               2
#define STATE_GET_ATTR              3
#define STATE_GET_VALUE             4
#define STATE_GET_VALUE_LENGTH      5 
#define STATE_GET_CUSTOM            6
#define STATE_GET_CUSTOM_LENGTH     7
#define STATE_GET_CUSTOM_ATTR       8
#define STATE_GET_ATTR_LENGTH       9
#define STATE_GET_HREF_LENGTH      10
#define STATE_GET_HREF             11

#define WRITE_CHAR(c) { \
    ch = c;\
    status = htmlwrite(write_data,&ch,1);\
    if (status < 0) goto write_error;\
    }

#define WRITE_STRING(s) { \
    status = htmlwrite(write_data,s,strlen(s));\
    if (status < 0) goto write_error;\
    }
   
/** EVAPORATED HTML *********************************************************
 
 Assumed to start in text, although all documents should start with a 
 tag. 
 Basically each name and element are 0-deliminated.

 so, roughly:  Text<0><flag>name<0>attr-name<0>attr-value<0><flag><0> 
 
 where flag:
    1xxx  - This is inside (and inclusive) of the <HEAD> element> 
    x1xx  - "Block" tags.  Varies between verious of the DLL though.
    xx1x  - Closing Tag
    xxx1  - Opening tag
    xx11  - Empty Tag   (should be <TAG />
   1xxxx  - Use a name from "atoms" array

 There's more for custom tag names and attributes.

 ***************************************************************************/
#define FLAG_OPENING    1
#define FLAG_CLOSING    2
#define FLAG_BLOCK      4
#define FLAG_HEAD	8
#define FLAG_ATOM      16

/*--[lit_reconstitute_html]----------------------------------------------------
 |
 | I read UTF8 from the buffer, and expand the internal compressed element
 | representation used by the IHtmlElement interface. 
 | 
 | I am recursive to handle nested tags, and return the number of bytes 
 | consumed.  
 | 
 |
 | Whitespace
 | This has been the source for bugs in every previous version of CLIT,
 | so I am rewriting how this is handled. 
 |
 | Key principle: Don't throw away whitespace from the file, since that
 | leads to irritation. Any "missing" whitespace can be added back by Tidy
 | anyway.
 |
 | Rules for XML/XHTML whitespace
 | A.   Whitespace is SPACE, TAB, CR, LF 
 | B.   Sequences of one or more whitespace characters are turned into one
 | C.   white space immediately after a tag should be ignored
 | D.   white space immediately before a end tag should be ignored
 | (E)  MSXML converts all whitespace into ' '
 |
 | Rule D doesn't really seem to work as I expect though. 
 | 
 | Rule B doesn't apply cases where I am inside xml:space="preserve" tags.
 | Since I don't want to scan for attributes, this means I cannot arbitrarily
 | add spaces except at the beginning and end!
 |
 | <BLOCK-TAG-1>Anything
 | should always be convertable to:
 | <BLOCK-TAG-1>
 | \t\tAnything
 | since all initial whitespace is ignored!
 | 
 | To avoid doing this for <I>,</I> type tags, I'll use blocking. This is 
 | a "convenience" choice -- <I>        is still rendered as <I>text !
 |                              text 
 | Complications:
 |    /meta (html_type = 1) doesn't use the upper 2 bits - force everything 
 |    into "block" mode. 
 |
 | The "RIGHT" solution would use HTMLTidy - but that wouldn't be in following
 | with the "unzip" style approach
 | 
 */
static int depth = 0;
static int pending_indent = 0;
static int was_in_text    = 0;  /* had non-whitespace text in the last block */

/* Kludge due to wierd recursion model.  I'm not going to rewrite that to fix
 * spacing. */
static int lingering_space = 0; 


int lit_reconstitute_html(U8 * pHtml, int base, int nBytes, int html_type,
    manifest_type * pManifest, lit_atom_list * pAtoms, 
    HTMLWRITEFCN htmlwrite, void * write_data ) 
{

    int                 elsize, state, index;
    int                 i, status, tag_index, char_count, href_base;
    U32                 c, flags, tag;
    int                 is_goingdown = 0; 
    attrmap             * current_map = NULL, * tmp_map;
    char                * tag_name = NULL, ch, numbuf[20];
    int                 dynamic_tag = 0;
  
    U32                 nTagRefs;
    char                ** mapTag2Name;
    attrmap             ** mapTag2Attrs;
    attrmap             * mapAttrs;
    int                 errors = 0;
    int                 in_censorship = 0;
    int                 space_enabled = 1, saved_space_enabled = 0;
  
    /* Was this tag indented? If so, newline/indent the end */
    int                 was_indented  = 0; 

    status = char_count = tag_index = flags = href_base = 0;

    /* Reinitialize stateful variables only the first time */
    if (!depth) {
        was_in_text = pending_indent = lingering_space = 0;
    }

    switch (html_type)
    {
    case 1:
            nTagRefs    = (U32)meta_tagcount;
            mapTag2Name = (char **)meta_tagtoname;
            mapTag2Attrs= (attrmap **)meta_tagtoattr;
            mapAttrs    = (attrmap *)meta_attr;
            break;
    default:
            nTagRefs    = (U32)tagcount;
            mapTag2Name = (char **)tagtoname;
            mapTag2Attrs= (attrmap **)tagtoattr;
            mapAttrs    = (attrmap *)tagmap0;
            break;
    }
    state = STATE_TEXT;
    index = base;

    while (index < (int)nBytes) {
        elsize = read_utf8_char(pHtml+index,(int)nBytes-index,&c);
        if (elsize < 1) {
            lit_error(ERR_R, "Invalid UTF8 character at "
                "position %d, %d bytes remain.", 
                index, (int)nBytes - index);
            return E_LIT_BAD_UTF8;
        }
/** printf("\nS: %02d Char: %ld, %lx, %c    (Depth = %d)\n",
            state, c, c, c, depth); **/
        switch (state) {
        case STATE_TEXT:
            if (!c) { state = STATE_GET_FLAGS; break; }  

            /* Treat as  xml:space="preserve" for the moment,
             * so the first white space ends expansion */
            if ((!was_in_text) || (space_enabled))
            {
                space_enabled = 0;
                if ((c == ' ') || (c == '\t') || (c == '\n') || 
                    (c == '\r'))  space_enabled++;
                else 
                    was_in_text = 1;
            }
            /* Special case, CTRL-K becomes a literal linefeed,
             * only in <PRE></PRE> blocks */
            if (c == '\v') c = '\n'; 

            /* Anything, even whitespace, negates this */
            pending_indent = 0;
               
            status = write_entity(c, htmlwrite, write_data);
            if (status < 0) goto write_error;
            break;
        case STATE_GET_FLAGS:
            if (!c) { state = STATE_TEXT; break; }
            flags = c;
#if 0
/* This is a don't care -- worse that happens is the output will be 
 * wierd. */
            if ((c > 15) || ((c & 0x03) == 0)) {
                lit_error(ERR_R,
"Invalid \"flags\" byte (0x%lx) at position %ld.\n"
"\tA flag byte ranges from 0x01 to 0x0f, except 0x04,0x08,0x0C.\n",
                c, index);
            } 
#endif
#if 0
            {
                char buf[32];
                sprintf(buf,"<!-- FLAG:%d%d%d%d%d%d%d%d-%d -->", 
                    (flags>>7)&1,(flags>>6)&1,
                    (flags>>5)&1,(flags>>4)&1,
                    (flags>>3)&1,(flags>>2)&1,
                    (flags>>1)&1,flags&1, in_censorship); 
                WRITE_STRING(buf);
            }
#endif
            state = STATE_GET_TAG;
            break;
        case STATE_GET_TAG:
            if (!c) { state = STATE_TEXT; }
            else state = STATE_GET_ATTR;
 
            if (flags & FLAG_OPENING) {

                /* Can do pending indents if spaces are enabled... */
                if (space_enabled)
                {
                   if ((!was_in_text) || (flags & (FLAG_BLOCK|FLAG_HEAD)))
                   {
                      pending_indent++;
                   } 
                }
                /* 
                 | 1.Pending indents..
                 | 2. /meta Always generates an 
                 |    extra newline at the beginning -- good thing there
                 |    is a DTD there! 
                 */
                if ((pending_indent) || (html_type == 1))
                {
                    was_indented++;
                    WRITE_CHAR('\n');
                   
#if 1 
                    for (i = 0; i < depth; i++)
                        WRITE_CHAR(' ');  
#endif
                    pending_indent = 0;
                }
                /* This sets up an indent for the next tag. 
                 * This MUST be cancelled as soon as any text happens.
                 * See rule C above */
                if ((flags & FLAG_HEAD) || (flags & FLAG_BLOCK)||
                    (html_type == 1) || (depth == 0))  {
                        pending_indent = 1; 
                }

                tag = c;

                WRITE_CHAR('<'); 

                if (!(flags & FLAG_CLOSING)) is_goingdown = 1; 
                if (tag == 0x8000) {
                    state = STATE_GET_CUSTOM_LENGTH;

                    break; 
                }
                /* Tag is a 1-based index into the atom names list */
                if (flags & FLAG_ATOM) {
                    if (!pAtoms || !tag || (tag > pAtoms->num_atoms)) { 
                        lit_error(ERR_R,
"Error - Custom tag %d at %ld isn't in Atom List (%08lx:%d)\n",
                            tag,index,pAtoms,pAtoms?pAtoms->num_atoms:0);	
                        return -1;
                    }
                    tag_name = pAtoms->atom_names[tag-1];
                    current_map = pAtoms->attrmap;
                }
                else if ((tag < nTagRefs) && (mapTag2Name[tag])) {
                    tag_name = mapTag2Name[tag]; 
                    current_map = mapTag2Attrs[tag];
                } else {
                    tag_name = malloc(20);
                    if (!tag_name) goto malloc_error;
                    dynamic_tag++;
                    sprintf(tag_name, "?%ld?", tag);

                    /* FUTUREFIX - This should be a "warning"?  */
                    lit_error(ERR_R,
"Unknown or unrecognized tag %08lx at position %ld.\n"
"\tCurrent depth is %d. ", tag, index, depth);
                    errors++;
                       
                    current_map = mapTag2Attrs[tag];
                } 
                WRITE_STRING(tag_name);
            } else if (flags & FLAG_CLOSING) {
                if (!depth) {
                    lit_error(ERR_R,
"Unbalanced HTML - extra ending tag at position %ld.",index);
                    return -1;
                }
                lingering_space = space_enabled;
                return ((index + elsize) - base);
            }
            break;
        case STATE_GET_ATTR:
            in_censorship = 0;

            if (!c) {
                if (!is_goingdown)  {
                    /* no need for tag_name anymore */
                    if (dynamic_tag && tag_name) free(tag_name); 
                    tag_name = NULL; 
                    dynamic_tag = 0;
                    WRITE_CHAR(' '); 
                    WRITE_CHAR('/');
                    WRITE_CHAR('>');
                } 
                else  /* is_going_down */
                {
                    WRITE_CHAR('>'); 
 
#if 0
                    WRITE_CHAR('[');
                    WRITE_CHAR('0'+space_enabled);
                    WRITE_CHAR(']');
#endif

                    if ((html_type == 0) && (flags & (FLAG_BLOCK|FLAG_HEAD))) 
                        pending_indent++;

                    if (depth > 1000) {
                        lit_error(ERR_R,
"Attempted to recurse too deeply at position %ld.", index);
                        return -2;
                    }
                    depth++; 
                    status = lit_reconstitute_html(pHtml,index+elsize,
                        (int)nBytes-elsize, html_type, pManifest, pAtoms,
			htmlwrite, write_data);
                      
                    depth--;
                    if (status < 0) return status;
                    index += status;
                    is_goingdown = 0;
                    if (!tag_name) {
                        lit_error(ERR_R,
"Unbalanced HTML - tag ends before it begins at position %ld.", index);
                        return -1;
                    }

                    /* Borrow the space enabled from the recursed routine
                     * until the tag is finished.  At that point, use the
                     * one that was saved... */
                    saved_space_enabled = space_enabled;
                    space_enabled = lingering_space; 

#if 0
                    WRITE_CHAR('(');
                    WRITE_CHAR('0'+space_enabled);
                    WRITE_CHAR('0'+saved_space_enabled);
                    WRITE_CHAR(')');
#endif
                    /* Can safely insert newlines here. 
                     * (Rule D) 
                     * Should only be for block tags though, and
                     * Conveince - don't do this if we were in text. 
                     *  (looks better...) 
                     * ** OOPS ** Seems like I _can't_. 
                     *  (unless rule B applies...) 
                     *  (and <PRE> be damned!)
                     */
                    if (space_enabled && was_indented  && (!was_in_text))
                    {
                        WRITE_CHAR('\n');
#if 1
                        for (i = 0; i < depth; i++) 
                            WRITE_CHAR(' ');
#endif
                    }
                    WRITE_CHAR('<');
                    WRITE_CHAR('/');
                    WRITE_STRING(tag_name);
                    WRITE_CHAR('>');

                    /* New lines here as well... But only with spaces !*/
                    if ( space_enabled &&
                        ((html_type == 1)||(flags & (FLAG_BLOCK|FLAG_HEAD))))
                    { 
                        pending_indent++;
                    }
                    if (dynamic_tag) free(tag_name); 
                    dynamic_tag = 0;
                    tag_name = NULL;

                    space_enabled = saved_space_enabled;
                }
                was_in_text = 0;
                state = STATE_TEXT;
                break;
            } else {
                i = 0;
                tmp_map = current_map;

                if (c == 0x8000) {
                    state = STATE_GET_ATTR_LENGTH;
                    break;
                }

                while (tmp_map && tmp_map->id)
                {
                    if (tmp_map->id == c)
                        break; 
                    tmp_map++;
                }
                if ((!tmp_map) || (!tmp_map->id))
                {
                    tmp_map = &mapAttrs[0]; 
                    while (tmp_map && tmp_map->id)
                    {
                        if (tmp_map->id == c) break;
                        tmp_map++;
                    }
                }


                if ((!tmp_map) || (!tmp_map->id)) {
                    lit_error(ERR_R,
"Unrecognized attribute (0x%08lx) at position %ld.\n"
"\tCurrently processing tag: \"%s\".  Depth is %d.  ",
                        c, index, tag_name, depth);
                    sprintf(numbuf,"?%ld?",c);
                    WRITE_CHAR(' ');
                    WRITE_STRING(numbuf);
                }
                else if (tmp_map->name[0] == '%') {
                    in_censorship = 1;
                    /* Invisible tag starting */
                    state = STATE_GET_VALUE_LENGTH;
                    break;
                }
                else {   
                    WRITE_CHAR(' ');
                    WRITE_STRING((char *)tmp_map->name);
                }
                if (status < 0) goto write_error;

                WRITE_CHAR('=');

                /* Two special cases. */
                
                if (tmp_map && tmp_map->name && 
                    ((strcmp(tmp_map->name,"href") == 0) || 
                    (strcmp(tmp_map->name,"src") == 0))) {
                    state = STATE_GET_HREF_LENGTH;
                    break;
                }
                state = STATE_GET_VALUE_LENGTH;
            }
            break;
        case STATE_GET_VALUE_LENGTH:
            if (!in_censorship) {
                WRITE_CHAR('\"');
            }
            char_count = (int)c - 1;
            if (!char_count) {
                if (!in_censorship) {
                    WRITE_CHAR('\"'); 
                }
                in_censorship = 0;
                state = STATE_GET_ATTR;
                break;
            }
            state = STATE_GET_VALUE;
            if (c == 0xffff) break;
            if ((char_count < 0) || (char_count > ((int)nBytes - index))) {
                lit_error(ERR_R,
"attribute had invalid length (%ld) at position %ld.",c, index);
                return -1;
            }
            break;
        case STATE_GET_VALUE:
            if (char_count == 0xfffe) {
                if (!in_censorship) {
                    /* Yes! There is no opening quote. */
                    sprintf(numbuf,"%ld\"",c-1);
                    WRITE_STRING(numbuf); 
                }
                in_censorship = 0;
                state = STATE_GET_ATTR; 
            }
            else if (char_count) {
                if (!in_censorship) { 
                    status = write_entity(c, htmlwrite, write_data);
                    if (status < 0) goto write_error; 
                }
                char_count--; 

            }
            if (!char_count) {
                if (!in_censorship) {
                    WRITE_CHAR('\"');
                }
                in_censorship = 0;
                state = STATE_GET_ATTR;
            } 
            break;
        case STATE_GET_CUSTOM_LENGTH:
            char_count = c - 1;
            if ((char_count <= 0) || (char_count > ((int)nBytes - index))) {
                lit_error(ERR_R,
"custom element had invalid length (%ld) at position %ld.", c, index);
                return -1;
            }
            tag_index = 0; 
            tag_name = malloc(char_count+1);
            dynamic_tag++;
            if (!tag_name)  goto malloc_error;
            state = STATE_GET_CUSTOM;
            break;
        case STATE_GET_CUSTOM:

            /* Yes, this throws away data. 
             | I hope that UTF8 isn't valid in tags, otherwise this will
             | result in unexpected behavior. */
            tag_name[tag_index++] = (char)(c&0x7F);
            char_count--;
            if (!char_count) {
                tag_name[tag_index] = '\0';
                WRITE_STRING(tag_name);
                state = STATE_GET_ATTR;
            }
            break;  
        case STATE_GET_ATTR_LENGTH:
            char_count = c - 1;
            if ((char_count <= 0) || (char_count > ((int)nBytes - index))) {
                lit_error(ERR_R,
"custom attribute had invalid length (%ld) at position %ld.",
                c, index);
                return -1;
            }
            WRITE_CHAR(' ');
            state = STATE_GET_CUSTOM_ATTR;
            break;
        case STATE_GET_CUSTOM_ATTR:
            status = write_entity(c, htmlwrite, write_data);
            if (status < 0) goto write_error;
            char_count--;
            if (!char_count) {
                WRITE_CHAR('=');
                state = STATE_GET_VALUE_LENGTH;
            }
            break;
        case STATE_GET_HREF_LENGTH:
            char_count = c - 1;
            if ((char_count <= 0) || (char_count > ((int)nBytes - index))) {
                lit_error(ERR_R,
"HREF tag has invalid length (%ld) at position %ld.\n", c, index);
                return -1;
            }
            href_base = index + elsize;
            state = STATE_GET_HREF;
            break;
        case STATE_GET_HREF:
            char_count--;
            if (!char_count) {
                int href_size;
                U8 * href_value, * hash_ptr, * new_href;

                href_size = (index + elsize) - href_base - 1;
                href_value = malloc(href_size + 1);
                if (!href_value) goto malloc_error;
    
                memcpy(href_value,pHtml + href_base + 1, href_size);
                href_value[href_size] = '\0';

                hash_ptr = strchr(href_value,'#');
                if (hash_ptr) 
                    new_href = lit_lookup_mapping(pManifest, href_value, 
                        hash_ptr - href_value);
                else 
                    new_href = lit_lookup_mapping(pManifest, href_value, 
                        href_size);
                WRITE_CHAR('\"');
                if (new_href) {
                    WRITE_STRING(new_href);
                    if (hash_ptr) {
                        WRITE_STRING(hash_ptr);
                    }

                } else {
                    WRITE_STRING(href_value); 
                }
                WRITE_CHAR('\"');
                free(href_value);
                state = STATE_GET_ATTR;
            }
            break;  
        default:
            lit_error(ERR_R,
"Reached an invalid internal state (%d) at position %ld.\n", state, index);
            return -1;
            break;
        }
        index += elsize;
    } 
    lingering_space = space_enabled;
    return (index - base);

write_error:
    lit_error(ERR_R, "Unexpected write error!");
    /* the htmlwrite routine should return a status value */
    return status;
malloc_error:
    lit_error(ERR_R,"Ran out of memory processing an HTML file!");
    return E_LIT_OUT_OF_MEMORY;
 
} 


/*--[write_entity]------------------------------------------------------------
 |
 | This converts an entity into a string representation fit for display, 
 | and writes it out to the callback.
 | 
 | This function returns the number of chars written
 */
int write_entity(U32 c, HTMLWRITEFCN htmlwrite, void * write_data)
{
    int     i, found;
    int     len, status;
    char    ent_buffer[14], ch;


    found = -1;

    for (i = 0; i < sizeof(entities)/sizeof(entities[0]); i++)
    {
        if (entities[i].id == c) { found = i; break;}
    }
    len = 0;
    if (found > -1)
    {
        ch = '&';
        status = htmlwrite(write_data,&ch, 1);
        if (status < 0) return status;
        len += status;

        status = htmlwrite(write_data,entities[found].name,
            strlen(entities[found].name));
        if (status < 0) return status;
        len += status;
        ch = ';';
        status = htmlwrite(write_data,&ch, 1);
        if (status < 0) return status;
        return (len + status);
    }
    else {
        if (c < 0x80) {
            ent_buffer[0] = (char)(c & 0x7f);
            len = 1;
        }
        else  {
            /* 32 bit value, so assumed to never be more than 10! */
            len = sprintf(ent_buffer,"&#%ld;",c);
        }
        return htmlwrite(write_data,ent_buffer,len);
    }
#if 0
/* I don't know if I should ever be writing out raw UTF8. */
        unsigned long int mask;
        mask = (1 << 11) - 1;
        len = 1;
        while (c & ~mask) {
            len++;
            mask = ((mask << 5) - 1)|(1 << 5);
        }
        status = fputc((0xff << (7 - len)) | (c >> (6*len)),fh);
        if (status < 0) return status;
        while (len) {
            status = fputc(0x80 | ((c >> (6*(--len))) & 0x3F), fh);
            if (status < 0) return status;
        }
    }
#endif
    return 0;
}


/*--[read_utf8_char]-----------------------------------------------------------
 |
 | This reads a single UTF8 character from a data stream, returning the
 | number of bytes consumed (element size) and filling in the Integer 
 | value. 
*/
int  read_utf8_char(U8 * pdata, int nBytes, U32 * pvalue)
{
    U32  c;
    unsigned char mask;
    int elsize, i;

    if (pvalue) *pvalue = -1;

    if (nBytes < 1) return -1;
    c = *(pdata);
    mask = 0x80;
    if (c & mask) {
        elsize = 0;
        while (c & mask) { mask >>= 1; elsize++;}
        if ((mask <= 1) || (mask == 0x40))  return -1;
    } else {
        elsize = 1;
    }

    if (elsize > 1) {
        if ((elsize) > nBytes) { return -1; }
        c &= (mask - 1);
        for (i = 1; i < elsize; i++) {
            if ( (*(pdata + i) & 0xC0) != 0x80)  return -1;
            c = (c << 6) | ( *(pdata + i) & 0x3F );
        }
    }
    if (pvalue) *pvalue = c;
    return elsize;       
}
