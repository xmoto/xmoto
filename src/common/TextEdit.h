#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include <stddef.h>
#include <string>
#include <utility> // for std::pair

class TextEdit {
public:
  TextEdit();
  ~TextEdit();

  static std::pair<std::string, size_t> deleteWordLeft(const std::string &str, size_t cursorPos);
  static std::pair<std::string, size_t> deleteWordRight(const std::string &str, size_t cursorPos);

  static size_t jumpWordLeft(const std::string &str, size_t cursorPos);
  static size_t jumpWordRight(const std::string &str, size_t cursorPos);
};

#endif // __TEXTEDIT_H__
