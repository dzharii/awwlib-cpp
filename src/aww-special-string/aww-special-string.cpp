#include "aww-special-string/aww-special-string.hpp"

namespace aww {
std::string safe_filename_from_string(std::string_view input) {
  std::string sanitized;
  sanitized.reserve(input.size());

  bool previous_was_replacement = false;

  const auto character_is_unsafe = [](char ch) -> bool {
    bool unsafe_on_linux = is_filename_char_unsafe_on_linux(ch);
    bool unsafe_on_windows = is_filename_char_unsafe_on_windows(ch);
    return unsafe_on_linux || unsafe_on_windows;
  };

  const auto has_trailing_dot_or_space = [](const std::string& text) -> bool {
    bool text_is_not_empty = !text.empty();
    bool trailing_dot_or_space = text_is_not_empty && (text.back() == '.' || text.back() == ' ');
    return trailing_dot_or_space;
  };

  for (unsigned char byte : input) {
    char ch = static_cast<char>(byte);

    if (character_is_unsafe(ch)) {
      if (!previous_was_replacement) {
        sanitized.push_back('_');
        previous_was_replacement = true;
      }
    } else {
      sanitized.push_back(ch);
      previous_was_replacement = false;
    }
  }

  while (has_trailing_dot_or_space(sanitized)) {
    sanitized.pop_back();
  }

  if (sanitized.empty() || sanitized == "_") {
    sanitized = std::string{};
  }

  return sanitized;
}
} // namespace aww
