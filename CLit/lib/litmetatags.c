
/*--[litmetatags.h]------------------------------------------------------------
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

/* Note - the '<' in an attribute means that it gets censored.  This 
 | resolves the "issue" of writing out an invalid OPF file.  */

#include "littags.h" 

attrmap meta_attr[] = { 
	{0x0001, "href"},	/* Never seen this, reported anonymously */
	{0x0002, "%never-used"},/* Never seen this either */
	{0x0003, "%guid"},
	{0x0004, "%minimum_level"},
	{0x0005, "%attr5"},
	{0x0006, "id"},
	{0x0007, "href"},
	{0x0008, "media-type"},
	{0x0009, "fallback"},
	{0x000A, "idref"},
	{0x000B, "xmlns:dc"},
	{0x000C, "xmlns:oebpackage"},
	{0x000D, "role"},
	{0x000E, "file-as"},
	{0x000F, "event"},
	{0x0010, "scheme"},
	{0x0011, "title"},
	{0x0012, "type"},
	{0x0013, "unique-identifier"},
	{0x0014, "name"},
	{0x0015, "content"},
        {0x0016, "xml:lang"},
	{0,0}};

char * meta_tagtoname[43] = {
 0,
        "package",
        "dc:Title",
        "dc:Creator",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        "manifest",
        "item",
        "spine",
        "itemref",
        "metadata",
        "dc-metadata",
        "dc:Subject",
        "dc:Description",
        "dc:Publisher",
        "dc:Contributor",
        "dc:Date",
        "dc:Type",
        "dc:Format",
        "dc:Identifier",
        "dc:Source",
        "dc:Language",
        "dc:Relation",
        "dc:Coverage",
        "dc:Rights",
        "x-metadata",
        "meta",
        "tours",
        "tour",
        "site",
        "guide",
        "reference",
        NULL };

attrmap * meta_tagtoattr[43] = {0};
const int meta_tagcount = 43;
