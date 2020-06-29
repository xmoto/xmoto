#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include <stddef.h>
#include <string>
#include <utility> // for std::pair

typedef std::pair<std::string, size_t> TextEditOP;

class TextEdit {
public:
  TextEdit();
  ~TextEdit();

  static TextEditOP deleteWordLeft(const std::string &str, size_t cursorPos);
  static TextEditOP deleteWordRight(const std::string &str, size_t cursorPos);

  static size_t jumpWordLeft(const std::string &str, size_t cursorPos);
  static size_t jumpWordRight(const std::string &str, size_t cursorPos);

  static TextEditOP insertAt(const std::string &str, const std::string &add, size_t at);
};

#endif // __TEXTEDIT_H__
