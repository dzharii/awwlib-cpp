#include "aww-string/aww-string.hpp"

namespace aww {

/**
 * @brief Splits a string into tokens based on a delimiter. (aww tag #y91j5qa0cn8)
 */
void string_trim_inplace(std::string& str) {
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
            str.end());
}

/**
 * @brief Trims whitespace from the end of a string. (aww tag #o4qbhyieany)
 */
void string_trim_right_inplace(std::string& str) {
  str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
            str.end());
}

/**
 * @brief Joins the elements of a vector into a single string with a specified delimiter. (aww tag #oibw1sg0jpv)
 */
[[nodiscard]] std::string join_vector(const std::vector<std::string_view>& collection, std::string_view delimiter) {
  // This ensures that vector size is greater than zero,
  // this makes v.size() - 1 to be safe.
  if (collection.empty()) {
    return std::string{};
  }
  size_t total_size = delimiter.size() * (collection.size() - 1);
  for (const auto& item : collection) {
    total_size += item.size();
  }

  // preallocate resulting string
  std::string out{};

  // Append elements with delimiter
  bool first = true;
  for (const auto& item : collection) {
    if (!first) {
      out.append(delimiter);
    }
    out.append(item);
    first = false;
  }
  return out;
}

} // namespace aww