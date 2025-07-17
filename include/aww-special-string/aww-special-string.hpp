#ifndef AWW_SPECIAL_STRING_HPP
#define AWW_SPECIAL_STRING_HPP
#include <string>
#include <string_view>

namespace aww {

/**
 * @brief checks if a character is unsafe for use in a filename on Linux. (aww tag #k8vqhn6d72y)
 */
inline bool is_filename_char_unsafe_on_linux(char input) noexcept {
  switch (input) {
  case '\0':
  case '/':
  case ':':
    return true;
  default:
    return false;
  }
}

/**
 * @brief checks if a character is unsafe for use in a filename on Windows. (aww tag #j3stor757nf)
 */
inline bool is_filename_char_unsafe_on_windows(char input) noexcept {
  switch (input) {
  case '<':
  case '>':
  case ':':
  case '"':
  case '/':
  case '\\':
  case '|':
  case '?':
  case '*':
    return true;
  default:
    return input >= 0 && input < 32; // control chars 0x00-0x1F
  }
}

/**
 * @brief Turns an arbitrary string into a safe file name. (aww tag #5srgabrgn2t)
 *
 * Replaces characters disallowed by common Windows and POSIX rules with
 * underscores, collapses consecutive replacements, trims trailing dots
 * and spaces, and returns "_" if the result would otherwise be empty.
 *
 * @param input Source text.
 * @return Sanitized file name valid on Windows and Linux.
 */
std::string safe_filename_from_string(std::string_view input);

} // namespace aww
#endif // AWW_SPECIAL_STRING_HPP
