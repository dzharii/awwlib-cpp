#include "aww-special-string/aww-special-string.hpp"

namespace aww {
std::string safe_filename_from_string(std::string_view input, unsigned char replacement_character) {
  bool input_string_is_the_replacement_character = (input.size() == 1 && input[0] == replacement_character);

  if (input_string_is_the_replacement_character) {
    return std::string{};
  }
  if (input.empty()) {
    return std::string{};
  }

  // Real work starts here
  std::string sanitized;
  sanitized.reserve(input.size());

  // Track whether the previous character was a replacement to avoid consecutive replacements
  bool previous_was_replacement = false;

  // Lambda to check if a character is unsafe on either Linux or Windows
  const auto character_is_unsafe = [](char ch) -> bool {
    bool unsafe_on_linux = is_filename_char_unsafe_on_linux(ch);
    bool unsafe_on_windows = is_filename_char_unsafe_on_windows(ch);
    return unsafe_on_linux || unsafe_on_windows;
  };

  // Lambda to check if the string ends with dots or spaces (Windows restriction)
  const auto has_trailing_dot_or_space = [](const std::string& text) -> bool {
    bool text_is_not_empty = !text.empty();
    bool trailing_dot_or_space = text_is_not_empty && (text.back() == '.' || text.back() == ' ');
    return trailing_dot_or_space;
  };

  // Process each character in the input string
  for (unsigned char byte : input) {
    char ch = static_cast<char>(byte);

    if (character_is_unsafe(ch)) {
      // Replace unsafe characters, but collapse consecutive unsafe chars into single replacement
      if (!previous_was_replacement) {
        sanitized.push_back(replacement_character);
        previous_was_replacement = true;
      }
    } else {
      // Safe character - add it directly
      sanitized.push_back(ch);
      previous_was_replacement = false;
    }
  }

  // Remove trailing dots and spaces (Windows file system requirement)
  while (has_trailing_dot_or_space(sanitized)) {
    sanitized.pop_back();
  }

  // Return empty string if result is empty or consists only of the replacement character
  if (sanitized.empty()) {
    sanitized = std::string{};
  }

  if (sanitized.length() == 1 && sanitized[0] == replacement_character) {
    // If the result is a single replacement character, return an empty string
    sanitized = std::string{};
  }

  return sanitized;
}
} // namespace aww
