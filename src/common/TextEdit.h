#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include <stddef.h>
#include <string>

class TextEdit {
public:
  TextEdit(bool multiline = false, bool hidden = false)
    : m_cursorPos(0)
    , m_multiline(multiline)
    , m_hidden(hidden) {}
  ~TextEdit() {}

  void clear();

  size_t calculateWordLeft() const;
  size_t calculateWordRight() const;

  void deleteLeft(int count = 1);
  void deleteRight(int count = 1);

  void deleteWordLeft();
  void deleteWordRight();

  void jumpWordLeft();
  void jumpWordRight();

  void jumpTo(size_t position);
  void jumpToStart();
  void jumpToEnd();

  size_t insert(const std::string &str);
  size_t insertAt(const std::string &str, size_t at);
  size_t insertFromClipboard();

  bool isHidden() const { return m_hidden; }
  void setHidden(bool hidden) { m_hidden = hidden; };

  inline std::string text() const { return m_text; }
  inline size_t cursorPos() const { return m_cursorPos; }

  void setText(const std::string &text) { m_text = text; }
  void moveCursor(int amt);

private:
  std::string m_text;
  size_t m_cursorPos;
  bool m_multiline;
  bool m_hidden;
};

#endif // __TEXTEDIT_H__
