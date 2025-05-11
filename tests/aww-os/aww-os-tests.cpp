#include "aww-os/aww-os.hpp"
#include "doctest/doctest.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <cstdlib>
#define setenv(name, value, overwrite) _putenv_s(name, value)
#define unsetenv(name) _putenv((std::string(name) + "=").c_str())
#else
#include <stdlib.h>
#endif

namespace fs = std::filesystem;

// Utility function to setup test environment
void setup_environment_for_test_read_raw_configuration_file(const std::string& test_dir, const std::string& test_file,
                                                            const std::string& content) {
  fs::path exec_path = aww::get_current_executable_path();
  fs::path test_path = exec_path.parent_path() / test_dir;

  // Create test directory if it does not exist
  if (!fs::exists(test_path)) {
    fs::create_directory(test_path);
  }

  // Create and write to the test configuration file
  fs::path file_path = test_path / test_file;
  std::ofstream out(file_path);
  out << content;
  out.close();
}

TEST_CASE("getenv") {
  SUBCASE("Set environment variable") {
    const std::string env_var = "TEST_ENV_VAR";
    const char* env_value = "Test Value";
    setenv(env_var.c_str(), env_value, 1);

    std::optional<std::string> result = aww::getenv(env_var);

    CHECK(result.has_value());
    CHECK(result.value() == env_value);

    unsetenv(env_var.c_str());
  }

  SUBCASE("Unset environment variable") {
    const char* env_var = "TEST_ENV_VAR";
    const char* env_value = "Test Value";
    setenv(env_var, env_value, 1);

    std::optional<std::string> result = aww::getenv(env_var);
    CHECK(result.has_value());
    CHECK(result.value() == env_value);

    unsetenv(env_var);
    result = aww::getenv(env_var);
    CHECK(!result.has_value());
  }

  SUBCASE("Get non-existent environment variable") {
    const char* env_var = "NON_EXISTENT_VAR";
    std::optional<std::string> result = aww::getenv(env_var);
    CHECK(!result.has_value());
  }
}

TEST_CASE("Test get_current_executable_path returns a valid path") {
  auto path = aww::get_current_executable_path();
  CHECK_FALSE(path.empty());            // Path should not be empty
  CHECK(std::filesystem::exists(path)); // The path should exist
}

TEST_CASE("Test get_current_executable_path returns the correct executable path") {
  auto path = aww::get_current_executable_path();
  // The below check assumes the test executable name is known and consistent
  bool correctFilename = path.filename() == "unit_tests" || path.filename() == "unit_tests.exe";
  std::string message = "The executable filename is not correct: " + path.filename().string();

  CHECK_MESSAGE(correctFilename, message);
}

TEST_CASE("has_redirected_standard_input: Standard input attached to terminal") {
  // When standard input is attached to a terminal, this function should return false.
  // Note: In some automated environments (e.g. CI systems), this may already be redirected.
  CHECK(aww::has_redirected_standard_input() == false);
}

TEST_CASE("has_redirected_standard_input: Standard input redirected") {
  // Create a temporary file to simulate redirected input.
  std::string tmp_file_name = "temp_stdin.txt";
  {
    std::ofstream ofs(tmp_file_name);
    ofs << "Sample input for testing.\n";
  }

  // RAII guard to restore standard input and clean up the temporary file.
  struct stdin_restore {
    std::string file_name;
    stdin_restore(const std::string& fn) : file_name(fn) {
    }
    ~stdin_restore() {
#ifdef _WIN32
      freopen("CON", "r", stdin);
#else
      freopen("/dev/tty", "r", stdin);
#endif
      std::remove(file_name.c_str());
    }
  } guard(tmp_file_name);

  // Redirect stdin to the temporary file.
  FILE* file = freopen(tmp_file_name.c_str(), "r", stdin);
  REQUIRE(file != nullptr);

  // Verify that the function correctly detects redirected standard input.
  CHECK(aww::has_redirected_standard_input() == true);
}

TEST_CASE("get_user_home_folder returns a valid path if HOME is set") {
#ifdef _WIN32
  const char* env_var = "USERPROFILE";
#else
  const char* env_var = "HOME";
#endif
  auto env_value = aww::getenv(env_var);
  auto home_path_opt = aww::get_user_home_folder();
  REQUIRE(home_path_opt.has_value());
  auto home_path = home_path_opt.value();
  CHECK(std::filesystem::exists(home_path));
  if (env_value.has_value()) {
    CHECK(home_path == std::filesystem::absolute(std::filesystem::path(env_value.value())));
  }
#ifdef _WIN32
  CHECK(home_path.string().find("Users") != std::string::npos);
#else
  CHECK(home_path.string().find("home") != std::string::npos);
#endif
}

TEST_CASE("get_user_home_folder returns a valid path if HOME is not set (Linux only)") {
#ifndef _WIN32
  // Save and unset HOME
  auto orig_home = aww::getenv("HOME");
  unsetenv("HOME");
  auto home_path_opt = aww::get_user_home_folder();
  REQUIRE(home_path_opt.has_value());
  auto home_path = home_path_opt.value();
  CHECK(std::filesystem::exists(home_path));
  CHECK(home_path.string().find("home") != std::string::npos);
  // Restore HOME
  if (orig_home.has_value()) {
    setenv("HOME", orig_home.value().c_str(), 1);
  }
#endif
}
