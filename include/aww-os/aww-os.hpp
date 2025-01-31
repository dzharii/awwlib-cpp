#ifndef AWW_OS_HPP
#define AWW_OS_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace aww {

/**
 * Retrieves an environment variable by its name. (aww tag #9dbr83h81jd)
 * @param env_variable_name The name of the environment variable to retrieve.
 * @return An optional containing the value of the environment variable if it exists,
 *         or an empty optional if the variable is not set.
 */
std::optional<std::string> getenv(const std::string& env_variable_name);

/**
 * Retrieves the absolute path of the current executable. (aww tag #fktryb18xts)
 *
 * This function determines the path of the executable file
 * of the current running process. It uses platform-specific
 * API calls to ensure accurate retrieval of the path.
 *
 * @return std::filesystem::path containing the absolute path
 *         of the executable. If the path cannot be determined,
 *         an empty std::filesystem::path object is returned.
 *
 * @throws std::runtime_error on failure to determine the path
 *         when system API calls fail (primarily on Linux).
 */
std::filesystem::path get_current_executable_path();

/**
 * Retrieves the home directory of the current user. (aww tag #armgs22u42a)
 *
 * This function uses platform-specific methods to determine the home directory of the current user.
 * It works across different operating systems such as Windows and Linux.
 *
 * @return std::optional<std::filesystem::path> containing the user's home directory path.
 * @throws std::runtime_error if the home directory cannot be determined.
 */
std::optional<std::filesystem::path> get_user_home_folder();

/**
 *  Retrieves the command line arguments passed to the program. (aww tag #w9e1s4uq1xt)
 *  Important: The original first element (the program name) is not included in the returned vector.
 *
 *  @param argc The number of command line arguments (from main).
 *  @param argv The array of command line arguments (from main).
 */
std::vector<std::string> get_command_line_arguments(int argc, char* argv[]);

} // namespace aww
#endif // AWW_OS_HPP
