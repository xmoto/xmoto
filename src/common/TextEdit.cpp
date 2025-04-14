#include "TextEdit.h"
#include "helpers/System.h"
#include "helpers/VMath.h"
#include "helpers/utf8.h"

/* NOTE: This probably does not work properly with utf8 yet */

void TextEdit::clear() {
  m_cursorPos = 0;
  m_text.clear();
}

void TextEdit::deleteWordLeft() {
  size_t oldPos = m_cursorPos;
  size_t newPos = m_cursorPos - calculateWordLeft();

  m_text =
    utf8::utf8_substring(m_text, 0, newPos) +
    utf8::utf8_substring(m_text, oldPos, utf8::utf8_length(m_text) - oldPos);

  m_cursorPos = newPos;
}

void TextEdit::deleteWordRight() {
  const size_t bound = std::string::npos;

  size_t deleteTo = m_cursorPos + calculateWordRight();

  m_text = utf8::utf8_substring(m_text, 0, m_cursorPos) +
           utf8::utf8_substring(
             m_text, deleteTo, utf8::utf8_length(m_text) - deleteTo);
}

void TextEdit::deleteRight(int count) {
  for (int i = 0; i < count; i++) {
    if (m_cursorPos + i > utf8::utf8_length(m_text))
      break;

    m_text = utf8::utf8_delete(m_text, m_cursorPos + 1);
  }
}

void TextEdit::deleteLeft(int count) {
  for (int i = 0; i < count; i++) {
    if (m_cursorPos < 1)
      break;

    m_text = utf8::utf8_delete(m_text, m_cursorPos);
    --m_cursorPos;
  }
}

void TextEdit::jumpWordLeft() {
  m_cursorPos -= calculateWordLeft();
}

void TextEdit::jumpWordRight() {
  m_cursorPos += calculateWordRight();
}

void TextEdit::jumpTo(size_t position) {
  m_cursorPos = std::min<size_t>(position, utf8::utf8_length(m_text));
}

void TextEdit::jumpToStart() {
  m_cursorPos = 0;
}

void TextEdit::jumpToEnd() {
  m_cursorPos = utf8::utf8_length(m_text);
}

size_t TextEdit::insertAt(const std::string &i_str, size_t at) {
  std::string str = i_str;

  if (!m_multiline) {
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  }

  if (at >= m_text.length()) {
    m_text = utf8::utf8_concat(m_text, str);
  } else {
    m_text = utf8::utf8_insert(m_text, str, at);
  }

  return utf8::utf8_length(str);
}

size_t TextEdit::insertFromClipboard() {
  return insert(System::getClipboardText());
}

size_t TextEdit::insert(const std::string &str) {
  size_t numInserted = insertAt(str, m_cursorPos);
  m_cursorPos += numInserted;
  return numInserted;
}

void TextEdit::moveCursor(int amt) {
  m_cursorPos = (size_t)clamp<int>((int)m_cursorPos + amt, 0, m_text.length());
}

size_t TextEdit::calculateWordLeft() const {
  if (m_hidden) {
    return m_cursorPos; // not valid with utf8
  }

  const size_t bound = 0;

  size_t wordStart = m_text.find_last_not_of(" ", m_cursorPos - 1);
  if (wordStart < bound)
    return m_cursorPos - bound;

  if (wordStart != std::string::npos) {
    wordStart = m_text.find_last_of(" ", wordStart);

    if (wordStart < bound)
      return m_cursorPos - bound;
  }

  if (wordStart == std::string::npos)
    wordStart = bound;
  else
    wordStart = wordStart + 1;

  return m_cursorPos - wordStart;
}

size_t TextEdit::calculateWordRight() const {
  if (m_hidden) {
    return utf8::utf8_length(m_text) - m_cursorPos;
  }

  const size_t bound = std::string::npos;

  size_t wordEnd = m_text.find_first_not_of(" ", m_cursorPos);

  if (wordEnd != std::string::npos) {
    wordEnd = m_text.find_first_of(" ", wordEnd);

    if (wordEnd > bound)
      return bound - m_cursorPos;
  }

  if (wordEnd == std::string::npos)
    return utf8::utf8_length(m_text) - m_cursorPos;

  return wordEnd - m_cursorPos;
}
