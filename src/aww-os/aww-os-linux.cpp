#include "aww-os/aww-os.hpp"
#include <limits.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace aww {
/**
 *  @brief Retrieves the absolute path of the current executable. (aww tag #fktryb18xts)
 */
std::filesystem::path get_current_executable_path() {
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::filesystem::path(std::string(result, (count > 0) ? count : 0));
}

/**
 * @brief Retrieves the home directory of the current user. (aww tag #armgs22u42a)
 */
std::optional<std::filesystem::path> get_user_home_folder() {
  auto home = aww::getenv("HOME");

  if (home.has_value()) {
    return std::filesystem::path(home.value());
  }

  struct passwd* pw = getpwuid(getuid());
  if (pw != nullptr) {
    return std::filesystem::path(pw->pw_dir);
  }

  return std::nullopt;
}

/**
 * @brief Retrieves the command line arguments passed to the program. (aww tag #w9e1s4uq1xt). The original first element
 * (the program name) is not included in the returned vector.
 */
std::vector<std::string> get_command_line_arguments(int argc, char* argv[]) {
  std::vector<std::string> cmdArgs(argv, argv + argc);
  cmdArgs.erase(cmdArgs.begin());
  return cmdArgs;
}

} // namespace aww
