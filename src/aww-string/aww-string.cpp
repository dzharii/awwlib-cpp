#include "aww-string/aww-string.hpp"

namespace aww {

/**
 * @brief Splits a string into tokens based on a delimiter. (aww tag #y91j5qa0cn8)
 */
void string_trim_inplace(std::string& str) {
  str.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                      [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(
      std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); })
          .base(),
      str.end());
}

/**
 * @brief Trims whitespace from the end of a string. (aww tag #o4qbhyieany)
 */
void string_trim_right_inplace(std::string& str) {
  str.erase(
      std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); })
          .base(),
      str.end());
}

}