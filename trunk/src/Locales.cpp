/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#define PACKAGE_LANG "xmoto"

std::string Locales::init(std::string i_locale) {
#ifdef USE_GETTEXT
  char *locale;
  char* btd;

#ifdef WIN32
  /* gettext at 0.13 - not enought for LC_MESSAGE */
  /* LC_CTYPE seems to work */
  locale = setlocale(LC_CTYPE, i_locale.c_str());
#else
  locale = setlocale(LC_MESSAGES, i_locale.c_str());
#endif

  if(locale == NULL) {
    return "";
  }


  textdomain (PACKAGE_LANG);
  if((btd=bindtextdomain (PACKAGE_LANG, LOCALESDIR)) == NULL) {
    return "";
  }

  return locale;

#endif
  return "";
}

