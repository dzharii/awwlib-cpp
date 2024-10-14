#include "aww-string/aww-string.hpp"

namespace aww {

// #y91j5qa0cn8
void string_trim_inplace(std::string& str) {
  str.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                      [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(
      std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); })
          .base(),
      str.end());
}

// #o4qbhyieany
std::string string_trim_right(std::string str) {
  str.erase(
      std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); })
          .base(),
      str.end());
  return str;
}

}