#include "doctest/doctest.h"
#include "aww-os/aww-os.hpp"

#ifdef _WIN32
#include <cstdlib>
#define setenv(name, value, overwrite) _putenv_s(name, value)
#define unsetenv(name) _putenv((std::string(name) + "=").c_str())
#else
#include <stdlib.h>
#endif

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