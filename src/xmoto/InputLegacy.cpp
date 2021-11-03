#include "InputLegacy.h"

void InputSDL12Compat::resolveConflicts(const std::vector<IFullKey *> &keys) {
  bool done;
  do {
    done = true;

    for (int i = 0; i < keys.size(); ++i) {
      if (!keys[i]->key.isBound())
        keys[i]->key = keys[i]->defaultKey;

      for (int j = 0; j < keys.size(); ++j) {
        if (i == j)
          continue;

        if (keys[i]->key == keys[j]->key) {
          keys[i]->key = keys[i]->defaultKey;
          keys[j]->key = keys[j]->defaultKey;
          done = false;
        }
      }
    }
  } while (!done);
}

void InputSDL12Compat::mapKey(XMKey &key, const Keytable &map) {
  auto it = map.find(key.getKeyboardSym());

  SDL_Keycode keycode = key.getKeyboardSym();
  /* handle SDL1.2 "world keys" (SDLK_WORLD_0..SDLK_WORLD_95) */
  if (keycode >= 160 && keycode <= 255) {
    key = XMKey();
    return;
  }

  if (it != map.end()) {
    // key modifiers are the same between SDL 1.2 and 2.0,
    // no need to do anything about them
    key = XMKey(it->second, key.getKeyboardMod());
  }
}

void InputSDL12Compat::upgrade() {
  size_t size = sizeof(SDL12_KEYTABLE)/sizeof(*SDL12_KEYTABLE);
  Keytable map;
  map.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    auto &p = SDL12_KEYTABLE[i];
    map.insert(std::pair<int32_t, int32_t>(p[0], p[1]));
  }

  std::vector<IFullKey *> keys;

  for (auto &f : InputHandler::instance()->m_globalControls) {
    if (f.customizable)
      keys.push_back(&f);
  }
  for (int player = 0; player < INPUT_NB_PLAYERS; ++player) {
    for (auto &f : InputHandler::instance()->m_controls[player].playerKeys) {
      if (f.customizable)
        keys.push_back(&f);
    }
    for (auto &f : InputHandler::instance()->m_controls[player].scriptActionKeys) {
      if (f.customizable)
        keys.push_back(&f);
    }
  }

  for (auto &f : keys)
    mapKey(f->key, map);

  resolveConflicts(keys);
}

bool InputSDL12Compat::isUpgraded(xmDatabase *pDb, const std::string &i_id_profile) {
  return !pDb->config_getBool(i_id_profile, "KeyCompatUpgrade", true);
}
