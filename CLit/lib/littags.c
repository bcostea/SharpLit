
/*--[littags.c]----------------------------------------------------------------
 | Copyright (C) 2002  Dan A. Jackson 
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
#include "littags.h" 

attrmap tagmap0[] = { 
	{0x8010, "tabindex"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x804d, "disabled"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x83fe, "datafld"},
	{0x83ff, "datasrc"},
	{0x8400, "dataformatas"},
	{0x87d6, "accesskey"},
	{0x9392, "lang"},
	{0x93ed, "language"},
	{0x93fe, "dir"},
	{0x9771, "onmouseover"},
	{0x9772, "onmouseout"},
	{0x9773, "onmousedown"},
	{0x9774, "onmouseup"},
	{0x9775, "onmousemove"},
	{0x9776, "onkeydown"},
	{0x9777, "onkeyup"},
	{0x9778, "onkeypress"},
	{0x9779, "onclick"},
	{0x977a, "ondblclick"},
	{0x977e, "onhelp"},
	{0x977f, "onfocus"},
	{0x9780, "onblur"},
	{0x9783, "onrowexit"},
	{0x9784, "onrowenter"},
	{0x9786, "onbeforeupdate"},
	{0x9787, "onafterupdate"},
	{0x978a, "onreadystatechange"},
	{0x9790, "onscroll"},
	{0x9794, "ondragstart"},
	{0x9795, "onresize"},
	{0x9796, "onselectstart"},
	{0x9797, "onerrorupdate"},
	{0x9799, "ondatasetchanged"},
	{0x979a, "ondataavailable"},
	{0x979b, "ondatasetcomplete"},
	{0x979c, "onfilterchange"},
	{0x979f, "onlosecapture"},
	{0x97a0, "onpropertychange"},
	{0x97a2, "ondrag"},
	{0x97a3, "ondragend"},
	{0x97a4, "ondragenter"},
	{0x97a5, "ondragover"},
	{0x97a6, "ondragleave"},
	{0x97a7, "ondrop"},
	{0x97a8, "oncut"},
	{0x97a9, "oncopy"},
	{0x97aa, "onpaste"},
	{0x97ab, "onbeforecut"},
	{0x97ac, "onbeforecopy"},
	{0x97ad, "onbeforepaste"},
	{0x97af, "onrowsdelete"},
	{0x97b0, "onrowsinserted"},
	{0x97b1, "oncellchange"},
	{0x97b2, "oncontextmenu"},
	{0x97b6, "onbeforeeditfocus"},
	{0,0}};
attrmap tagmap3[] = { 
	{0x0001, "href"},
	{0x03ec, "target"},
	{0x03ee, "rel"},
	{0x03ef, "rev"},
	{0x03f0, "urn"},
	{0x03f1, "methods"},
	{0x8001, "name"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap5[] = { 
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap6[] = { 
	{0x8001, "name"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x804a, "align"},
	{0x8bbb, "classid"},
	{0x8bbc, "data"},
	{0x8bbf, "codebase"},
	{0x8bc0, "codetype"},
	{0x8bc1, "code"},
	{0x8bc2, "type"},
	{0x8bc5, "vspace"},
	{0x8bc6, "hspace"},
	{0x978e, "onerror"},
	{0,0}};
attrmap tagmap7[] = { 
	{0x0001, "href"},
	{0x03ea, "shape"},
	{0x03eb, "coords"},
	{0x03ed, "target"},
	{0x03ee, "alt"},
	{0x03ef, "nohref"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap8[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap9[] = { 
	{0x03ec, "href"},
	{0x03ed, "target"},
	{0,0}};
attrmap tagmap10[] = { 
	{0x938b, "color"},
	{0x939b, "face"},
	{0x93a3, "size"},
	{0,0}};
attrmap tagmap12[] = { 
	{0x03ea, "src"},
	{0x03eb, "loop"},
	{0x03ec, "volume"},
	{0x03ed, "balance"},
	{0,0}};
attrmap tagmap13[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap15[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap16[] = { 
	{0x07db, "link"},
	{0x07dc, "alink"},
	{0x07dd, "vlink"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938a, "background"},
	{0x938b, "text"},
	{0x938e, "nowrap"},
	{0x93ae, "topmargin"},
	{0x93af, "rightmargin"},
	{0x93b0, "bottommargin"},
	{0x93b1, "leftmargin"},
	{0x93b6, "bgproperties"},
	{0x93d8, "scroll"},
	{0x977b, "onselect"},
	{0x9791, "onload"},
	{0x9792, "onunload"},
	{0x9798, "onbeforeunload"},
	{0x97b3, "onbeforeprint"},
	{0x97b4, "onafterprint"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap17[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap18[] = { 
	{0x07d1, "type"},
	{0x8001, "name"},
	{0,0}};
attrmap tagmap19[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x93a8, "valign"},
	{0,0}};
attrmap tagmap20[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap21[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap22[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap23[] = { 
	{0x03ea, "span"},
	{0x8006, "width"},
	{0x8049, "align"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap24[] = { 
	{0x03ea, "span"},
	{0x8006, "width"},
	{0x8049, "align"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap27[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938e, "nowrap"},
	{0,0}};
attrmap tagmap29[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap31[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938e, "nowrap"},
	{0,0}};
attrmap tagmap32[] = { 
	{0x03ea, "compact"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap33[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938e, "nowrap"},
	{0,0}};
attrmap tagmap34[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap35[] = { 
	{0x8001, "name"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x804a, "align"},
	{0x8bbd, "palette"},
	{0x8bbe, "pluginspage"},
	{0x8bbf, "codebase"},
	{0x8bbf, "src"},
	{0x8bc1, "units"},
	{0x8bc2, "type"},
	{0x8bc3, "hidden"},
	{0,0}};
attrmap tagmap36[] = { 
	{0x804a, "align"},
	{0,0}};
attrmap tagmap37[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938b, "color"},
	{0x939b, "face"},
	{0x939c, "size"},
	{0,0}};
attrmap tagmap38[] = { 
	{0x03ea, "action"},
	{0x03ec, "enctype"},
	{0x03ed, "method"},
	{0x03ef, "target"},
	{0x03f4, "accept-charset"},
	{0x8001, "name"},
	{0x977c, "onsubmit"},
	{0x977d, "onreset"},
	{0,0}};
attrmap tagmap39[] = { 
	{0x8000, "align"},
	{0x8001, "name"},
	{0x8bb9, "src"},
	{0x8bbb, "border"},
	{0x8bbc, "frameborder"},
	{0x8bbd, "framespacing"},
	{0x8bbe, "marginwidth"},
	{0x8bbf, "marginheight"},
	{0x8bc0, "noresize"},
	{0x8bc1, "scrolling"},
	{0x8fa2, "bordercolor"},
	{0,0}};
attrmap tagmap40[] = { 
	{0x03e9, "rows"},
	{0x03ea, "cols"},
	{0x03eb, "border"},
	{0x03ec, "bordercolor"},
	{0x03ed, "frameborder"},
	{0x03ee, "framespacing"},
	{0x8001, "name"},
	{0x9791, "onload"},
	{0x9792, "onunload"},
	{0x9798, "onbeforeunload"},
	{0x97b3, "onbeforeprint"},
	{0x97b4, "onafterprint"},
	{0,0}};
attrmap tagmap42[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap43[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap44[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap45[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap46[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap47[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap49[] = { 
	{0x03ea, "noshade"},
	{0x8006, "width"},
	{0x8007, "size"},
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938b, "color"},
	{0,0}};
attrmap tagmap51[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap52[] = { 
	{0x8001, "name"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x804a, "align"},
	{0x8bb9, "src"},
	{0x8bbb, "border"},
	{0x8bbc, "frameborder"},
	{0x8bbd, "framespacing"},
	{0x8bbe, "marginwidth"},
	{0x8bbf, "marginheight"},
	{0x8bc0, "noresize"},
	{0x8bc1, "scrolling"},
	{0x8fa2, "vspace"},
	{0x8fa3, "hspace"},
	{0,0}};
attrmap tagmap53[] = { 
	{0x03eb, "alt"},
	{0x03ec, "src"},
	{0x03ed, "border"},
	{0x03ee, "vspace"},
	{0x03ef, "hspace"},
	{0x03f0, "lowsrc"},
	{0x03f1, "vrml"},
	{0x03f2, "dynsrc"},
	{0x03f4, "loop"},
	{0x03f6, "start"},
	{0x07d3, "ismap"},
	{0x07d9, "usemap"},
	{0x8001, "name"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x8046, "title"},
	{0x804a, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x978d, "onabort"},
	{0x978e, "onerror"},
	{0x9791, "onload"},
	{0,0}};
attrmap tagmap54[] = { 
	{0x07d1, "type"},
	{0x07d3, "size"},
	{0x07d4, "maxlength"},
	{0x07d6, "readonly"},
	{0x07d8, "indeterminate"},
	{0x07da, "checked"},
	{0x07db, "alt"},
	{0x07dc, "src"},
	{0x07dd, "border"},
	{0x07de, "vspace"},
	{0x07df, "hspace"},
	{0x07e0, "lowsrc"},
	{0x07e1, "vrml"},
	{0x07e2, "dynsrc"},
	{0x07e4, "loop"},
	{0x07e5, "start"},
	{0x8001, "name"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x804a, "align"},
	{0x93ee, "value"},
	{0x977b, "onselect"},
	{0x978d, "onabort"},
	{0x978e, "onerror"},
	{0x978f, "onchange"},
	{0x9791, "onload"},
	{0,0}};
attrmap tagmap56[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap57[] = { 
	{0x03e9, "for"},
	{0,0}};
attrmap tagmap58[] = { 
	{0x804a, "align"},
	{0,0}};
attrmap tagmap59[] = { 
	{0x03ea, "value"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x939a, "type"},
	{0,0}};
attrmap tagmap60[] = { 
	{0x03ee, "href"},
	{0x03ef, "rel"},
	{0x03f0, "rev"},
	{0x03f1, "type"},
	{0x03f9, "media"},
	{0x03fa, "target"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x978e, "onerror"},
	{0x9791, "onload"},
	{0,0}};
attrmap tagmap61[] = { 
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap62[] = { 
	{0x8001, "name"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap63[] = { 
	{0x1771, "scrolldelay"},
	{0x1772, "direction"},
	{0x1773, "behavior"},
	{0x1774, "scrollamount"},
	{0x1775, "loop"},
	{0x1776, "vspace"},
	{0x1777, "hspace"},
	{0x1778, "truespeed"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x9785, "onbounce"},
	{0x978b, "onfinish"},
	{0x978c, "onstart"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap65[] = { 
	{0x03ea, "http-equiv"},
	{0x03eb, "content"},
	{0x03ec, "url"},
	{0x03f6, "charset"},
	{0x8001, "name"},
	{0,0}};
attrmap tagmap66[] = { 
	{0x03f5, "n"},
	{0,0}};
attrmap tagmap71[] = { 
	{0x8000, "border"},
	{0x8000, "usemap"},
	{0x8001, "name"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x8046, "title"},
	{0x804a, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x8bbb, "classid"},
	{0x8bbc, "data"},
	{0x8bbf, "codebase"},
	{0x8bc0, "codetype"},
	{0x8bc1, "code"},
	{0x8bc2, "type"},
	{0x8bc5, "vspace"},
	{0x8bc6, "hspace"},
	{0x978e, "onerror"},
	{0,0}};
attrmap tagmap72[] = { 
	{0x03eb, "compact"},
	{0x03ec, "start"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x939a, "type"},
	{0,0}};
attrmap tagmap73[] = { 
	{0x03ea, "selected"},
	{0x03eb, "value"},
	{0,0}};
attrmap tagmap74[] = { 
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap75[] = { 
	{0x8000, "name"},
	{0x8000, "value"},
	{0x8000, "type"},
	{0,0}};
attrmap tagmap76[] = { 
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap77[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x9399, "clear"},
	{0,0}};
attrmap tagmap78[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap82[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap83[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap84[] = { 
	{0x03ea, "src"},
	{0x03ed, "for"},
	{0x03ee, "event"},
	{0x03f0, "defer"},
	{0x03f2, "type"},
	{0x978e, "onerror"},
	{0,0}};
attrmap tagmap85[] = { 
	{0x03eb, "size"},
	{0x03ec, "multiple"},
	{0x8000, "align"},
	{0x8001, "name"},
	{0x978f, "onchange"},
	{0,0}};
attrmap tagmap86[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap87[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap88[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap89[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap90[] = { 
	{0x03eb, "type"},
	{0x03ef, "media"},
	{0x8046, "title"},
	{0x978e, "onerror"},
	{0x9791, "onload"},
	{0,0}};
attrmap tagmap91[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap92[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap93[] = { 
	{0x03ea, "cols"},
	{0x03eb, "border"},
	{0x03ec, "rules"},
	{0x03ed, "frame"},
	{0x03ee, "cellspacing"},
	{0x03ef, "cellpadding"},
	{0x03fa, "datapagesize"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x8046, "title"},
	{0x804a, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938a, "background"},
	{0x93a5, "bordercolor"},
	{0x93a6, "bordercolorlight"},
	{0x93a7, "bordercolordark"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap94[] = { 
	{0x8049, "align"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap95[] = { 
	{0x8049, "align"},
	{0x93a8, "valign"},
	{0,0}};
attrmap tagmap96[] = { 
	{0x07d2, "rowspan"},
	{0x07d3, "colspan"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938a, "background"},
	{0x938e, "nowrap"},
	{0x93a5, "bordercolor"},
	{0x93a6, "bordercolorlight"},
	{0x93a7, "bordercolordark"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap97[] = { 
	{0x1b5a, "rows"},
	{0x1b5b, "cols"},
	{0x1b5c, "wrap"},
	{0x1b5d, "readonly"},
	{0x8001, "name"},
	{0x977b, "onselect"},
	{0x978f, "onchange"},
	{0,0}};
attrmap tagmap98[] = { 
	{0x8049, "align"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap99[] = { 
	{0x07d2, "rowspan"},
	{0x07d3, "colspan"},
	{0x8006, "width"},
	{0x8007, "height"},
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x938a, "background"},
	{0x938e, "nowrap"},
	{0x93a5, "bordercolor"},
	{0x93a6, "bordercolorlight"},
	{0x93a7, "bordercolordark"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap100[] = { 
	{0x8049, "align"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap102[] = { 
	{0x8007, "height"},
	{0x8046, "title"},
	{0x8049, "align"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x93a5, "bordercolor"},
	{0x93a6, "bordercolorlight"},
	{0x93a7, "bordercolordark"},
	{0x93a8, "valign"},
	{0xfe0c, "bgcolor"},
	{0,0}};
attrmap tagmap103[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap104[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap105[] = { 
	{0x03eb, "compact"},
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0x939a, "type"},
	{0,0}};
attrmap tagmap106[] = { 
	{0x8046, "title"},
	{0x804b, "style"},
	{0x83ea, "class"},
	{0x83eb, "id"},
	{0,0}};
attrmap tagmap108[] = { 
	{0x9399, "clear"},
	{0,0}};
char * tagtoname[109] = { 
 0, 
	NULL,
	NULL,
	"a",
	"acronym",
	"address",
	"applet",
	"area",
	"b",
	"base",
	"basefont",
	"bdo",
	"bgsound",
	"big",
	"blink",
	"blockquote",
	"body",
	"br",
	"button",
	"caption",
	"center",
	"cite",
	"code",
	"col",
	"colgroup",
	NULL,
	NULL,
	"dd",
	"del",
	"dfn",
	"dir",
	"div",
	"dl",
	"dt",
	"em",
	"embed",
	"fieldset",
	"font",
	"form",
	"frame",
	"frameset",
	NULL,
	"h1",
	"h2",
	"h3",
	"h4",
	"h5",
	"h6",
	"head",
	"hr",
	"html",
	"i",
	"iframe",
	"img",
	"input",
	"ins",
	"kbd",
	"label",
	"legend",
	"li",
	"link",
	"tag61",
	"map",
	"tag63",
	"tag64",
	"meta",
	"nextid",
	"nobr",
	"noembed",
	"noframes",
	"noscript",
	"object",
	"ol",
	"option",
	"p",
	"param",
	"plaintext",
	"pre",
	"q",
	"rp",
	"rt",
	"ruby",
	"s",
	"samp",
	"script",
	"select",
	"small",
	"span",
	"strike",
	"strong",
	"style",
	"sub",
	"sup",
	"table",
	"tbody",
	"tc",
	"td",
	"textarea",
	"tfoot",
	"th",
	"thead",
	"title",
	"tr",
	"tt",
	"u",
	"ul",
	"var",
	"wbr",
	NULL };

attrmap * tagtoattr[109] = { 
 0, 
	NULL,
	NULL,
	tagmap3, /* a */ 
	NULL, /* acronym */ 
	tagmap5, /* address */ 
	tagmap6, /* applet */ 
	tagmap7, /* area */ 
	tagmap8, /* b */ 
	tagmap9, /* base */ 
	tagmap10, /* basefont */ 
	NULL, /* bdo */ 
	tagmap12, /* bgsound */ 
	tagmap13, /* big */ 
	NULL, /* blink */ 
	tagmap15, /* blockquote */ 
	tagmap16, /* body */ 
	tagmap17, /* br */ 
	tagmap18, /* button */ 
	tagmap19, /* caption */ 
	tagmap20, /* center */ 
	tagmap21, /* cite */ 
	tagmap22, /* code */ 
	tagmap23, /* col */ 
	tagmap24, /* colgroup */ 
	NULL,
	NULL,
	tagmap27, /* dd */ 
	NULL, /* del */ 
	tagmap29, /* dfn */ 
	NULL, /* dir */ 
	tagmap31, /* div */ 
	tagmap32, /* dl */ 
	tagmap33, /* dt */ 
	tagmap34, /* em */ 
	tagmap35, /* embed */ 
	tagmap36, /* fieldset */ 
	tagmap37, /* font */ 
	tagmap38, /* form */ 
	tagmap39, /* frame */ 
	tagmap40, /* frameset */ 
	NULL,
	tagmap42, /* h1 */ 
	tagmap43, /* h2 */ 
	tagmap44, /* h3 */ 
	tagmap45, /* h4 */ 
	tagmap46, /* h5 */ 
	tagmap47, /* h6 */ 
	NULL, /* head */ 
	tagmap49, /* hr */ 
	NULL, /* html */ 
	tagmap51, /* i */ 
	tagmap52, /* iframe */ 
	tagmap53, /* img */ 
	tagmap54, /* input */ 
	NULL, /* ins */ 
	tagmap56, /* kbd */ 
	tagmap57, /* label */ 
	tagmap58, /* legend */ 
	tagmap59, /* li */ 
	tagmap60, /* link */ 
	tagmap61, /* tag61 */ 
	tagmap62, /* map */ 
	tagmap63, /* tag63 */ 
	NULL, /* tag64 */ 
	tagmap65, /* meta */ 
	tagmap66, /* nextid */ 
	NULL, /* nobr */ 
	NULL, /* noembed */ 
	NULL, /* noframes */ 
	NULL, /* noscript */ 
	tagmap71, /* object */ 
	tagmap72, /* ol */ 
	tagmap73, /* option */ 
	tagmap74, /* p */ 
	tagmap75, /* param */ 
	tagmap76, /* plaintext */ 
	tagmap77, /* pre */ 
	tagmap78, /* q */ 
	NULL, /* rp */ 
	NULL, /* rt */ 
	NULL, /* ruby */ 
	tagmap82, /* s */ 
	tagmap83, /* samp */ 
	tagmap84, /* script */ 
	tagmap85, /* select */ 
	tagmap86, /* small */ 
	tagmap87, /* span */ 
	tagmap88, /* strike */ 
	tagmap89, /* strong */ 
	tagmap90, /* style */ 
	tagmap91, /* sub */ 
	tagmap92, /* sup */ 
	tagmap93, /* table */ 
	tagmap94, /* tbody */ 
	tagmap95, /* tc */ 
	tagmap96, /* td */ 
	tagmap97, /* textarea */ 
	tagmap98, /* tfoot */ 
	tagmap99, /* th */ 
	tagmap100, /* thead */ 
	NULL, /* title */ 
	tagmap102, /* tr */ 
	tagmap103, /* tt */ 
	tagmap104, /* u */ 
	tagmap105, /* ul */ 
	tagmap106, /* var */ 
	NULL, /* wbr */ 
0};

const int tagcount = 109;
