#include "aww-os/aww-os.hpp"
#include "Windows.h"

namespace aww {
/**
 * @brief Retrieves the absolute path of the current executable.
 */
std::filesystem::path get_current_executable_path(void) {
  char buffer[MAX_PATH];
  DWORD nchar;
  nchar = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
  if (nchar == 0) {
    return std::filesystem::path("");
  }
  return std::filesystem::path(buffer);
}

/**
 * @brief Retrieves the home directory of the current user.
 */
std::optional<std::filesystem::path> get_user_home_folder() {
  auto home_drive = aww::getenv("HOMEDRIVE");
  auto home_path = aww::getenv("HOMEPATH");
  auto user_profile = aww::getenv("USERPROFILE");

  if (home_drive.has_value() && home_path.has_value()) {
    return std::filesystem::path(home_drive.value() + home_path.value());
  } else if (user_profile.has_value()) {
    return std::filesystem::path(user_profile.value());
  }
  return std::nullopt;
}
} // end of namespace