#pragma once
#ifndef AWW_STRING_HPP
#define AWW_STRING_HPP

#include <string>

namespace aww {

/**
 * @brief Trims whitespace from the beginning and end of a string. #y91j5qa0cn8
 *
 * @param str The string to trim.
 */
void string_trim_inplace(std::string& str);

/**
 * @brief Trims whitespace from the end of a string. #o4qbhyieany
 *
 * @param str The string to trim.
 * @return std::string The trimmed string.
 */
std::string string_trim_right(std::string str);


} // end of namespace
#endif // AWW_STRING_HPP
