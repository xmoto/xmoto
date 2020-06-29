#include "TextEdit.h"
#include "helpers/utf8.h"

/* NOTE: This stuff probably does not work with utf8 */

TextEditOP TextEdit::deleteWordLeft(const std::string &str, size_t cursorPos) {
  size_t newPos = cursorPos - TextEdit::jumpWordLeft(str, cursorPos);

  std::string newStr = utf8::utf8_substring(str, 0, newPos) +
                       utf8::utf8_substring(str, cursorPos, utf8::utf8_length(str) - cursorPos);
  return std::make_pair(newStr, newPos);
}

TextEditOP TextEdit::deleteWordRight(const std::string &str, size_t cursorPos) {
  size_t deleteTo = cursorPos + TextEdit::jumpWordRight(str, cursorPos);

  std::string newStr = utf8::utf8_substring(str, 0, cursorPos) +
                       utf8::utf8_substring(str, deleteTo, utf8::utf8_length(str) - deleteTo);

  return std::make_pair(newStr, cursorPos);
}


size_t TextEdit::jumpWordLeft(const std::string &str, size_t cursorPos) {
  size_t wordStart = str.find_last_not_of(" ", cursorPos-1);
  if (wordStart != std::string::npos) {
    wordStart = str.find_last_of(" ", wordStart);
  }

  if (wordStart == std::string::npos)
    wordStart = 0;
  else
    wordStart = wordStart + 1;

  return cursorPos - wordStart;
}

size_t TextEdit::jumpWordRight(const std::string &str, size_t cursorPos) {
  size_t wordEnd = str.find_first_not_of(" ", cursorPos);
  if (wordEnd != std::string::npos) {
    wordEnd = str.find_first_of(" ", wordEnd);
  }

  if (wordEnd == std::string::npos)
    return utf8::utf8_length(str) - cursorPos;
  else
    return wordEnd - cursorPos;
}

TextEditOP TextEdit::insertAt(const std::string &str, const std::string &add, size_t at) {
  std::string newStr =
    utf8::utf8_substring(str, 0, at) + add +
    utf8::utf8_substring(str, at, utf8::utf8_length(str) - at);

  at += utf8::utf8_length(add);

  return std::make_pair(newStr, at);
}

