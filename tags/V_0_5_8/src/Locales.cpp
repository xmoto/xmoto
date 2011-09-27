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

#include "Locales.h"
#include <iostream>
#include "BuildConfig.h"
#include "helpers/Environment.h"
#include "helpers/VExcept.h"

#define PACKAGE_LANG "xmoto"

std::string Locales::default_LANGUAGE;

#ifndef USE_GETTEXT
char* ngettext(char* msgid, char* msgid_plural, unsigned long int n) {
  if(n > 1) {
    return msgid_plural;
  }
  return msgid;
}
#endif

std::string Locales::changeLocale(const std::string& i_locale) {
  char *locale = NULL;

  try {
    if(i_locale != "") {
      // this var is looked by gettext in priority (and it is set on most environment, then change it to change the lang)
      Environment::set_variable("LANGUAGE", i_locale);
    } else {
      Environment::set_variable("LANGUAGE", default_LANGUAGE);
    }
  } catch(Exception &e) {
    /* hum, ok */
  }

#ifdef WIN32
    /* gettext at 0.13 - not enought for LC_MESSAGE */
    /* LC_CTYPE seems to work */
    locale = setlocale(LC_CTYPE, "");
#else
    locale = setlocale(LC_CTYPE, "");
    locale = setlocale(LC_MESSAGES, "");
#endif

  if(locale == NULL) {
    return "";
  }

  return locale;
}

std::string Locales::init(std::string i_locale) {
#ifdef USE_GETTEXT
  std::string locale;
  char* btd;
  char* cs;

  default_LANGUAGE = Environment::get_variable("LANGUAGE");

  locale = changeLocale(i_locale);

  if(locale == "") {
    return locale;
  }

  textdomain(PACKAGE_LANG);
  if((btd=bindtextdomain(PACKAGE_LANG, LOCALESDIR)) == NULL) {
    return "";
  }

  cs = bind_textdomain_codeset(PACKAGE_LANG, "UTF-8");

  return locale;

#endif
  return "";
}
