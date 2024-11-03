#ifndef AWW_COLLECTION_HPP
#define AWW_COLLECTION_HPP

#include <algorithm> // for std::erase
#include <vector>

namespace aww {

/**
 * @brief Removes all occurrences of a specified value from a vector. (aww tag #i2a5v9vagda)
 *
 * This function erases all instances of the provided `value` from the given `vec`.
 * It is particularly useful for removing specific flags from command-line arguments.
 *
 * **Use Case Example:**
 * ```cpp
 * std::vector<std::string> args = {"--verbose", "--help", "--verbose"};
 * bool removed = erase_all_elements(args, std::string("--verbose"));
 * // After removal, args contains: {"--help"}
 * // removed == true
 * ```
 *
 * @tparam T The type of elements stored in the vector.
 * @param vec The vector from which to remove elements.
 * @param value The value to remove from the vector.
 * @return `true` if one or more elements were erased; `false` otherwise.
 */
template <typename T> bool erase_all_matched_elements(std::vector<T>& vec, const T& value) { return std::erase(vec, value) > 0; } // namespace aww

} // namespace aww
#endif // AWW_COLLECTION_HPP
