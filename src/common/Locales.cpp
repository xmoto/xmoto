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
#include <libintl.h>
#include <iostream>
#include "BuildConfig.h"
#include "helpers/Environment.h"
#include "helpers/VExcept.h"
#include "helpers/Log.h"
#include "common/VFileIO.h"

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

std::pair<std::string, std::string> Locales::changeLocale(const std::string& i_locale) {
  if(i_locale != "") {
    // this var is looked by gettext in priority (and it is set on most environment, then change it to change the lang)
    Environment::set_variable("LANGUAGE", i_locale);
  } else {
    Environment::set_variable("LANGUAGE", default_LANGUAGE);
  }

  std::pair<const char*, const char*> locale {NULL, NULL};
#ifdef WIN32
    /* gettext at 0.13 - not enough for LC_MESSAGE */
    /* LC_CTYPE seems to work */
    locale.first = setlocale(LC_CTYPE, "");
#else
    LogInfo("Before CTYPE / MESSAGES to %s: %s / %s", i_locale.c_str(),
            setlocale(LC_CTYPE, NULL), setlocale(LC_MESSAGES, NULL));
    locale.first = setlocale(LC_CTYPE, i_locale.c_str());
    locale.second = setlocale(LC_MESSAGES, i_locale.c_str());
    LogInfo("After CTYPE / MESSAGES: %s / %s", setlocale(LC_CTYPE, NULL), setlocale(LC_MESSAGES, NULL));
#endif

  /* Make change known.  */
  {
    extern int _nl_msg_cat_cntr;
    ++_nl_msg_cat_cntr;
  }

  std::pair<std::string, std::string> locale_str (
    locale.first == NULL ? std::string("") : std::string(locale.first),
    locale.second == NULL ? std::string("") : std::string(locale.second)
  );

  return locale_str;
}

std::string Locales::init(std::string i_locale) {
#ifdef USE_GETTEXT
  std::pair<std::string, std::string> locale;
  char* btd;
  char* cs;

  default_LANGUAGE = Environment::get_variable("LANGUAGE");

  locale = changeLocale(i_locale);

  textdomain(PACKAGE_LANG);
  std::string locale_dir = XMFS::getSystemLocaleDir();
  LogInfo("%s (%s / %s): %s", i_locale.c_str(), locale.first.c_str(),
          locale.second.c_str(), locale_dir.c_str());
  if((btd=bindtextdomain(PACKAGE_LANG, locale_dir.c_str())) == NULL) {
    LogInfo("Bad bindtextdomain");
    return "";
  }

  cs = bind_textdomain_codeset(PACKAGE_LANG, "UTF-8");

  return locale.second;

#endif
  return "";
}
