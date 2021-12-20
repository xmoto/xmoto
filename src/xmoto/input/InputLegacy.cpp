#include "InputLegacy.h"

void InputSDL12Compat::resolveConflicts(const std::vector<IFullKey *> &keys) {
  bool done;
  do {
    done = true;

    for (int i = 0; i < keys.size(); ++i) {
      if (!keys[i]->key.isDefined())
        keys[i]->key = keys[i]->defaultKey;

      for (int j = 0; j < keys.size(); ++j) {
        if (i == j)
          continue;

        if (!keys[i]->customizable || !keys[j]->customizable)
          continue;

        if (keys[i]->key == keys[j]->key && keys[i]->key.isDefined()) {
          keys[i]->key = keys[i]->defaultKey;
          keys[j]->key = keys[j]->defaultKey;
          done = false;
        }
      }
    }
  } while (!done);
}

void InputSDL12Compat::mapKey(XMKey &key) {
  SDL_Keycode keycode = key.getKeyboardSym();
  // unbind SDL1.2 "world keys" (SDLK_WORLD_0..SDLK_WORLD_95),
  // since they don't have obvious counterparts in SDL2
  if (keycode >= 160 && keycode <= 255) {
    key = XMKey();
    return;
  }

  auto it = SDL12_KEYTABLE.find(key.getKeyboardSym());

  if (it != SDL12_KEYTABLE.end()) {
    // key modifiers are the same between SDL 1.2 and 2.0,
    // no need to do anything about them
    key = XMKey(it->second, key.getKeyboardMod());
  }
}

void InputSDL12Compat::upgrade() {
  std::vector<IFullKey *> keys;

  for (auto &f : Input::instance()->m_globalControls) {
    keys.push_back(&f);
  }
  for (int player = 0; player < INPUT_NB_PLAYERS; ++player) {
    for (auto &f : Input::instance()->m_controls[player].playerKeys) {
      keys.push_back(&f);
    }
    for (auto &f : Input::instance()->m_controls[player].scriptActionKeys) {
      keys.push_back(&f);
    }
  }

  for (auto &f : keys)
    mapKey(f->key);

  resolveConflicts(keys);
}

bool InputSDL12Compat::isUpgraded(xmDatabase *pDb, const std::string &i_id_profile) {
  return !pDb->config_getBool(i_id_profile, "KeyCompatUpgrade", true);
}
