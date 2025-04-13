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

#include "common/VXml.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "xmDatabase.h"

std::string xmDatabase::config_getValue(const std::string &i_id_profile,
                                        const std::string &i_key,
                                        unsigned int &o_nrow) {
  char **v_result;
  std::string v_res;

  v_result = readDB("SELECT value FROM profiles_configs "
                    "WHERE id_profile=\"" +
                      xmDatabase::protectString(i_id_profile) +
                      "\" "
                      "AND key=\"" +
                      xmDatabase::protectString(i_key) + "\";",
                    o_nrow);
  if (o_nrow != 1) {
    read_DB_free(v_result);
    return ""; /* not found */
  }
  v_res = getResult(v_result, 1, 0, 0);

  read_DB_free(v_result);
  return v_res;
}

void xmDatabase::config_setValue(const std::string &i_id_profile,
                                 const std::string &i_key,
                                 const std::string &i_value) {
  if (checkKey("SELECT count(1) FROM profiles_configs "
               "WHERE id_profile=\"" +
               xmDatabase::protectString(i_id_profile) +
               "\" "
               "AND   key=\"" +
               xmDatabase::protectString(i_key) + "\";")) {
    simpleSql("UPDATE profiles_configs SET "
              "value=\"" +
              xmDatabase::protectString(i_value) +
              "\" "
              "WHERE id_profile=\"" +
              xmDatabase::protectString(i_id_profile) +
              "\" "
              "AND          key=\"" +
              xmDatabase::protectString(i_key) + "\";");
  } else {
    simpleSql("INSERT INTO profiles_configs(id_profile, key, value) VALUES("
              "\"" +
              xmDatabase::protectString(i_id_profile) +
              "\", "
              "\"" +
              xmDatabase::protectString(i_key) +
              "\", "
              "\"" +
              xmDatabase::protectString(i_value) +
              "\""
              ");");
  }
}

void xmDatabase::config_setValue_begin() {
  simpleSql("BEGIN TRANSACTION;");
}

void xmDatabase::config_setValue_end() {
  simpleSql("COMMIT;");
}

std::string xmDatabase::config_getString(const std::string &i_id_profile,
                                         const std::string &i_key,
                                         const std::string &i_default) {
  std::string v_res;
  unsigned int nrow;

  v_res = config_getValue(i_id_profile, i_key, nrow);
  if (nrow != 1) {
    return i_default; /* not found */
  }
  return v_res;
}

bool xmDatabase::config_getBool(const std::string &i_id_profile,
                                const std::string &i_key,
                                bool i_default) {
  std::string v_res;
  unsigned int nrow;

  v_res = config_getValue(i_id_profile, i_key, nrow);
  if (nrow != 1) {
    return i_default; /* not found */
  }
  return (v_res == "1") ? true : false;
}

bool xmDatabase::config_keyExists(const std::string &i_id_profile,
                                  const std::string &i_key) {
  unsigned int nrow;
  config_getValue(i_id_profile, i_key, nrow);
  return nrow == 1;
}

int xmDatabase::config_getInteger(const std::string &i_id_profile,
                                  const std::string &i_key,
                                  int i_default) {
  std::string v_res;
  unsigned int nrow;

  v_res = config_getValue(i_id_profile, i_key, nrow);
  if (nrow != 1) {
    return i_default; /* not found */
  }
  return atoi(v_res.c_str());
}

float xmDatabase::config_getFloat(const std::string &i_id_profile,
                                  const std::string &i_key,
                                  float i_default) {
  std::string v_res;
  unsigned int nrow;

  v_res = config_getValue(i_id_profile, i_key, nrow);
  if (nrow != 1) {
    return i_default; /* not found */
  }
  return atof(v_res.c_str());
}

void xmDatabase::config_setString(const std::string &i_id_profile,
                                  const std::string &i_key,
                                  const std::string &i_value) {
  config_setValue(i_id_profile, i_key, i_value);
}

void xmDatabase::config_setBool(const std::string &i_id_profile,
                                const std::string &i_key,
                                bool i_value) {
  config_setValue(i_id_profile, i_key, i_value ? "1" : "0");
}

void xmDatabase::config_setInteger(const std::string &i_id_profile,
                                   const std::string &i_key,
                                   int i_value) {
  char cBuf[256];
  snprintf(cBuf, 256, "%i", i_value);
  config_setValue(i_id_profile, i_key, cBuf);
}

void xmDatabase::config_setFloat(const std::string &i_id_profile,
                                 const std::string &i_key,
                                 float i_value) {
  char cBuf[256];
  snprintf(cBuf, 256, "%f", i_value);
  config_setValue(i_id_profile, i_key, cBuf);
}

void xmDatabase::updateDB_config(const std::string &i_sitekey) {
  /* load removed values into all the profiles */
  XMLDocument v_xml;
  xmlNodePtr v_xmlElt;

  v_xml.readFromFile(FDT_CONFIG, "config.dat");
  v_xmlElt = v_xml.getRootNode("userconfig");
  if (v_xmlElt == NULL) {
    throw Exception("Unable to read xml file");
  }

  unsigned int nrow;
  char **v_result;
  std::string v_id_profile;

  for (xmlNodePtr pSubElem = XMLDocument::subElement(v_xmlElt, "var");
       pSubElem != NULL;
       pSubElem = XMLDocument::nextElement(pSubElem)) {
    std::string Name, Value;

    Name = XMLDocument::getOption(pSubElem, "name");
    Value = XMLDocument::getOption(pSubElem, "value");

    try {
      if (Name == "WebHighscores" || Name == "WWWPassword" ||
          Name == "Language" || Name == "Theme" || Name == "QSQualityMIN" ||
          Name == "QSQualityMAX" || Name == "QSDifficultyMIN" ||
          Name == "QSDifficultyMAX" || Name == "AudioEnable" ||
          Name == "AudioSampleRate" || Name == "AudioSampleBits" ||
          Name == "AudioChannels" || Name == "EngineSoundEnable" ||
          Name == "KeyDrive1" || Name == "KeyBrake1" ||
          Name == "KeyFlipLeft1" || Name == "KeyFlipRight1" ||
          Name == "KeyChangeDir1" || Name == "KeyDrive2" ||
          Name == "KeyBrake2" || Name == "KeyFlipLeft2" ||
          Name == "KeyFlipRight2" || Name == "KeyChangeDir2" ||
          Name == "KeyDrive3" || Name == "KeyBrake3" ||
          Name == "KeyFlipLeft3" || Name == "KeyFlipRight3" ||
          Name == "KeyChangeDir3" || Name == "KeyDrive4" ||
          Name == "KeyBrake4" || Name == "KeyFlipLeft4" ||
          Name == "KeyFlipRight4" || Name == "KeyChangeDir4" ||
          Name == "AutosaveHighscoreReplays" || Name == "NotifyAtInit" ||
          Name == "ShowMiniMap" || Name == "ShowEngineCounter" ||
          Name == "ContextHelp" || Name == "MenuMusic" || Name == "InitZoom" ||
          Name == "CameraActiveZoom" || Name == "DeathAnim" ||
          Name == "CheckHighscoresAtStartup" ||
          Name == "CheckNewLevelsAtStartup" ||
          Name == "ShowInGameWorldRecord" || Name == "WebConfAtInit" ||
          Name == "UseCrappyPack" || Name == "UseChildrenCompliant" ||
          Name == "EnableGhost" || Name == "GhostStrategy_MYBEST" ||
          Name == "GhostStrategy_THEBEST" ||
          Name == "GhostStrategy_BESTOFREFROOM" ||
          Name == "GhostStrategy_BESTOFOTHERROOMS" ||
          Name == "ShowGhostTimeDiff" || Name == "DisplayGhostInfo" ||
          Name == "HideGhosts" || Name == "GhostMotionBlur" ||
          Name == "MultiStopWhenOneFinishes" || Name == "ProxyType" ||
          Name == "ProxyServer" || Name == "ProxyAuthUser" ||
          Name == "ProxyAuthPwd" || Name == "MenuGraphics" ||
          Name == "GameGraphics" || Name == "ProxyPort" ||
          Name == "WebHighscoresNbRooms" || Name == "WebHighscoresIdRoom" ||
          Name == "WebHighscoresIdRoom1" || Name == "WebHighscoresIdRoom2" ||
          Name == "WebHighscoresIdRoom3") {
        if (i_sitekey == "") {
          v_result = readDB("SELECT id_profile FROM stats_profiles;", nrow);
        } else {
          v_result =
            readDB("SELECT id_profile FROM stats_profiles WHERE sitekey=\"" +
                     xmDatabase::protectString(i_sitekey) + "\";",
                   nrow);
        }
        for (unsigned int i = 0; i < nrow; i++) {
          v_id_profile = getResult(v_result, 1, i, 0);

          /* boolean values */
          if (Name == "WebHighscores" || Name == "AudioEnable" ||
              Name == "EngineSoundEnable" ||
              Name == "AutosaveHighscoreReplays" || Name == "NotifyAtInit" ||
              Name == "ShowMiniMap" || Name == "ShowEngineCounter" ||
              Name == "ContextHelp" || Name == "MenuMusic" ||
              Name == "InitZoom" || Name == "CameraActiveZoom" ||
              Name == "DeathAnim" || Name == "CheckHighscoresAtStartup" ||
              Name == "CheckNewLevelsAtStartup" ||
              Name == "ShowInGameWorldRecord" || Name == "WebConfAtInit" ||
              Name == "UseCrappyPack" || Name == "UseChildrenCompliant" ||
              Name == "EnableGhost" || Name == "GhostStrategy_MYBEST" ||
              Name == "GhostStrategy_THEBEST" ||
              Name == "GhostStrategy_BESTOFREFROOM" ||
              Name == "GhostStrategy_BESTOFOTHERROOMS" ||
              Name == "ShowGhostTimeDiff" || Name == "DisplayGhostInfo" ||
              Name == "HideGhosts" || Name == "GhostMotionBlur" ||
              Name == "MultiStopWhenOneFinishes") {
            config_setBool(v_id_profile, Name, Value == "true" ? 1 : 0);

            /* string values */
          } else if (Name == "WWWPassword" || Name == "Language" ||
                     Name == "Theme" || Name == "AudioChannels" ||
                     Name == "KeyDrive1" || Name == "KeyBrake1" ||
                     Name == "KeyFlipLeft1" || Name == "KeyFlipRight1" ||
                     Name == "KeyChangeDir1" || Name == "KeyDrive2" ||
                     Name == "KeyBrake2" || Name == "KeyFlipLeft2" ||
                     Name == "KeyFlipRight2" || Name == "KeyChangeDir2" ||
                     Name == "KeyDrive3" || Name == "KeyBrake3" ||
                     Name == "KeyFlipLeft3" || Name == "KeyFlipRight3" ||
                     Name == "KeyChangeDir3" || Name == "KeyDrive4" ||
                     Name == "KeyBrake4" || Name == "KeyFlipLeft4" ||
                     Name == "KeyFlipRight4" || Name == "KeyChangeDir4" ||
                     Name == "ProxyType" || Name == "ProxyServer" ||
                     Name == "ProxyAuthUser" || Name == "ProxyAuthPwd" ||
                     Name == "MenuGraphics" || Name == "GameGraphics" ||
                     Name == "WebHighscoresIdRoom" ||
                     Name == "WebHighscoresIdRoom1" ||
                     Name == "WebHighscoresIdRoom2" ||
                     Name == "WebHighscoresIdRoom3") {
            config_setString(v_id_profile, Name, Value);

            /* Integers */
          } else if (Name == "QSQualityMIN" || Name == "QSQualityMAX" ||
                     Name == "QSDifficultyMIN" || Name == "QSDifficultyMAX" ||
                     Name == "AudioSampleRate" || Name == "AudioSampleBits" ||
                     Name == "ProxyPort" || Name == "WebHighscoresNbRooms") {
            config_setInteger(v_id_profile, Name, atoi(Value.c_str()));
          }
        }
        read_DB_free(v_result);
      }
    } catch (Exception &e) {
      LogWarning("%s", e.getMsg().c_str());
    }
  }
}
