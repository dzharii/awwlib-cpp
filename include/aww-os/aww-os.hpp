#pragma once
#ifndef AWW_OS_HPP
#define AWW_OS_HPP

#include <string>
#include <optional>

namespace aww {

/**
 * Retrieves an environment variable by its name.
 * @param env_variable_name The name of the environment variable to retrieve.
 * @return An optional containing the value of the environment variable if it exists,
 *         or an empty optional if the variable is not set.
 */
std::optional<std::string> getenv(const std::string& env_variable_name);

} // end of namespace
#endif // AWW_OS_HPP
