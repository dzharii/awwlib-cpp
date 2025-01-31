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

// Function to convert UTF-16 wide string to UTF-8 string
// TODO: if this works, move it to aww-string
std::string utf16_to_utf8(const std::wstring& utf16) {
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (size_needed <= 0) {
    return {};
  }
  std::string utf8(size_needed - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, &utf8[0], size_needed, nullptr, nullptr);
  return utf8;
}

/**
 * @brief Retrieves the command line arguments passed to the program. (aww tag #w9e1s4uq1xt). The original first element
 * (the program name) is not included in the returned vector.
 */
std::vector<std::string> get_command_line_arguments([[maybe_unused]] int argc_unused,
                                                    [[maybe_unused]] char* argv_unused[]) {
  // Get the wide command line string
  LPWSTR commandLine = GetCommandLineW();

  // Parse the wide command line into arguments
  int argc;
  LPWSTR* argv_wide = CommandLineToArgvW(commandLine, &argc);

  if (!argv_wide) {
    throw std::runtime_error("Failed to parse command line arguments.");
  }

  // Convert wide arguments to UTF-8
  std::vector<std::string> utf8_args;
  for (int i = 0; i < argc; ++i) {
    utf8_args.push_back(utf16_to_utf8(argv_wide[i]));
  }

  // Free the memory allocated for wide arguments
  LocalFree(argv_wide);

  utf8_args.erase(utf8_args.begin());

  return utf8_args;
}

} // namespace aww
