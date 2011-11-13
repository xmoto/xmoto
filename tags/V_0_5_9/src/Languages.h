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
  {"Català",      "Catala",      "ca_ES"}, // analpaper at gmail dot com
  {"Èesky",       "Eesky",       "cs_CZ"}, // tomas.chvatal at gmail dot com
  {"Dansk",       "Dansk",       "da_DK"}, // kristianjagd at gmail dot com
  {"Deutsch",     "Deutsch",     "de_DE"}, // mothbox at gmx dot net
  {"American",    "American",    "en_US"},
  {"Español",     "Espanol",     "es_ES"}, // analpaper at gmail dot com
  {"Suomi",       "Suomi",       "fi_FI"}, // tuhoojabotti at gmail dot com
  {"Français",    "Francais",    "fr_FR"}, // nicolas.adenis.lamarre at gmail dot com
  {"Gallego",     "Galego",      "gl_ES"}, // adriyetichaves at gmail dot com
  {"Magyar",      "Magyar",      "hu_HU"}, // ttapecs at gmail dot com
  {"Italiano",    "Italiano",    "it_IT"}, // earcar at gmail dot com
  {"Latviešu",    "Latviesu",    "lv_LV"}, // parasti at gmail dot com
  {"Lietuvių",    "Lietuviu",    "lt_LT"}, // tadzikaz at gmail dot com
  {"Norsk",       "Norsk",       "nb_NO"},
  {"Norsk",       "Norsk",       "nn_NO"},
  {"Nederlands",  "Nederlands",  "nl_NL"}, // dyingmuppet at gmail dot com
  {"Polski",      "Polski",      "pl_PL"},
  {"Brasileiro",  "Brasileiro",  "pt_BR"}, // kkndrox at gmail dot com
  {"Portugues",   "Portugues",   "pt_PT"}, // smarquespt at gmail dot com
  {"Русский",     "Russian",     "ru_RU"}, // thecentury at gmail dot com
  {"Slovensky",   "Slovensky",   "sk_SK"}, // jose1711 at gmail dot com
  {"Svenska",     "Svenska",     "sv_SE"}, // terra.unknown at yahoo dot com
  {"Türkçe",       "Turkce",     "tr_TR"}, // ozbekanil at gmail dot com
  {"Taiwanese",   "繁體中文",      "zh_TW"}, // azazabc123 at gmail dot com
  {NULL, NULL, NULL}
};

#endif
