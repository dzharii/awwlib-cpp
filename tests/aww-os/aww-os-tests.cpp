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