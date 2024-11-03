#ifndef AWW_STRING_HPP
#define AWW_STRING_HPP

#include <algorithm>
#include <string>
#include <vector>

namespace aww {

/**
 * @brief Trims whitespace from the beginning and end of a string. (aww tag #y91j5qa0cn8)
 *
 * @param str The string to trim.
 */
void string_trim_inplace(std::string& str);

/**
 * @brief Trims whitespace from the end of a string. (aww tag #o4qbhyieany)
 *
 * @param str The string to trim.
 */
void string_trim_right_inplace(std::string& str);

/**
 * @brief Joins the elements of a vector into a single string with a specified delimiter. (aww tag #oibw1sg0jpv)
 * @param collection A vector of `std::string_view` elements to be joined.
 * @param delimiter A `std::string_view` that will be used to separate each element in the resulting string.
 * @return A `std::string` containing all the elements of `collection`, separated by `delimiter`.
 */
[[nodiscard]] std::string join_vector(const std::vector<std::string_view>& collection, std::string_view delimiter);

} // namespace aww
#endif // AWW_STRING_HPP
