#include "aww-os/aww-os.hpp"

namespace aww {
/**
 * @brief an environment variable by its name.
 */
std::optional<std::string> getenv(const std::string& env_variable_name) {
  const char* env_value = std::getenv(env_variable_name.c_str());
  return env_value != nullptr ? std::optional<std::string>(env_value) : std::nullopt;
}
} // end of namespace