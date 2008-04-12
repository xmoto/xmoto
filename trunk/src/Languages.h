/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#ifndef __LANGUAGES_H__
#define __LANGUAGES_H__

#define NB_LANGUAGES  19
#define LANGUAGE_NAME  0
#define LANGUAGE_CODE  1

char* LANGUAGES[NB_LANGUAGES][2] = {
  {"Català",     "ca_ES"},
  {"Èesky",      "cs_CZ"},
  {"Dansk",      "da_DK"},
  {"Deutsch",    "de_DE"},
  {"American",   "en_US"},
  {"Español",    "es_ES"},
  {"Suomi",      "fi_FI"},
  {"Français",   "fr_FR"},
  {"Italiano",   "it_IT"},
  {"Latviešu",   "lv_LV"},
  {"Norsk",      "nb_NO"},
  {"Norsk",      "nn_NO"},
  {"Polski",     "pl_PL"},
  {"Brasileiro", "pt_BR"},
  {"Portugues",  "pt_PT"},
  {"Русский",    "ru_RU"},
  {"Slovensky",  "sk_SK"},
  {"Svenska",    "sv_SE"}
};

#endif
