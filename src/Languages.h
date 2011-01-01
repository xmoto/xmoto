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

enum {LANGUAGE_NAME = 0, LANGUAGE_ASIAN_NAME = 1, LANGUAGE_CODE = 2};

const char* LANGUAGES[][3] = {
  {"Català",      "Catala",      "ca_ES"},
  {"Èesky",       "Eesky",       "cs_CZ"},
  {"Dansk",       "Dansk",       "da_DK"},
  {"Deutsch",     "Deutsch",     "de_DE"},
  {"American",    "American",    "en_US"},
  {"Español",     "Espanol",     "es_ES"},
  {"Suomi",       "Suomi",       "fi_FI"},
  {"Français",    "Francais",    "fr_FR"},
  {"Magyar",      "Magyar",      "hu_HU"},
  {"Italiano",    "Italiano",    "it_IT"},
  {"Latviešu",    "Latviesu",    "lv_LV"},
  {"Lietuvių",    "Lietuviu",    "lt_LT"},
  {"Norsk",       "Norsk",       "nb_NO"},
  {"Norsk",       "Norsk",       "nn_NO"},
  {"Nederlands",  "Nederlands",  "nl_NL"},
  {"Polski",      "Polski",      "pl_PL"},
  {"Brasileiro",  "Brasileiro",  "pt_BR"},
  {"Portugues",   "Portugues",   "pt_PT"},
  {"Русский",     "Russian",     "ru_RU"},
  {"Slovensky",   "Slovensky",   "sk_SK"},
  {"Svenska",     "Svenska",     "sv_SE"},
  {"Taiwanese",   "繁體中文",      "zh_TW"},
  {"Türkçe",       "Turkce",     "tr_TR"},
  {NULL, NULL, NULL}
};

#endif
