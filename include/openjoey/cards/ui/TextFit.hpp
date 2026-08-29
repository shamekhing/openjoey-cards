#pragma once
#include <raylib.h>
#include <string>

namespace openjoey::ui {

// Truncates `text` character by character until it fits `maxWidth` at
// `fontSize`, appending "~" when characters were dropped.
inline std::string fitText(const std::string &text, int maxWidth, int fontSize) {
  std::string out = text;
  while (!out.empty() && MeasureText(out.c_str(), fontSize) > maxWidth)
    out.pop_back();
  if (out.size() < text.size())
    out += "~";
  return out;
}

} // namespace openjoey::ui
