#include "Windows.h"
#include "aww-os/aww-os.hpp"

namespace aww {
/**
 * @brief Retrieves the absolute path of the current executable. (aww tag #fktryb18xts)
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
 * @brief Retrieves the home directory of the current user. (aww tag #armgs22u42a)
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
} // namespace aww