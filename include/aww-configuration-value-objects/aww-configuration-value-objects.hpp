#ifndef AWW_CONFIGURATION_VALUE_OBJECTS_HPP
#define AWW_CONFIGURATION_VALUE_OBJECTS_HPP

#include "aww-value-objects/aww-value-objects.hpp"
#include <optional>
#include <stdexcept>
#include <string>

namespace aww {
/****************************************************************************************
 *  COLORS
 ****************************************************************************************/
/**
 * @class hex_color
 * @brief Represents a color in hexadecimal format. (aww tag #erkzwpvzvap)
 *
 * This class validates and parses a hexadecimal color string, extracting the red, green, blue,
 * and optional alpha components.
 */
class hex_color : public aww::value_object<std::string> {
public:
  explicit hex_color(std::string value) : value_object(std::move(value)) {
    validate(this->value());
    parse(this->value());
  }

  unsigned long red() const {
    return m_red;
  }
  unsigned long green() const {
    return m_green;
  }
  unsigned long blue() const {
    return m_blue;
  }
  std::optional<unsigned long> alpha() const {
    return m_alpha;
  }

  static void validate(const std::string& value) {
    if (value.empty() || value[0] != '#') {
      throw std::invalid_argument("Hex color must start with '#'.");
    }
    if (value.size() != 7 && value.size() != 9) {
      throw std::invalid_argument("Hex color must be 7 or 9 characters long.");
    }
    for (size_t i = 1; i < value.size(); ++i) {
      if (!isxdigit(value[i])) {
        throw std::invalid_argument("Hex color contains invalid characters.");
      }
    }
  }

  static bool is_valid(const std::string& value) {
    try {
      validate(value);
      return true;
    } catch (...) {
      return false;
    }
  }

private:
  void parse(const std::string& value) {
    m_red = std::stoul(value.substr(1, 2), nullptr, 16);
    m_green = std::stoul(value.substr(3, 2), nullptr, 16);
    m_blue = std::stoul(value.substr(5, 2), nullptr, 16);
    if (value.length() == 9) {
      m_alpha = std::stoul(value.substr(7, 2), nullptr, 16);
    }
  }

  unsigned long m_red;
  unsigned long m_green;
  unsigned long m_blue;
  std::optional<unsigned long> m_alpha;
};

/**
 * @class background_color
 * @brief Represents the background color of a component. (aww tag #z9e5fhedyru)
 *
 * This class inherits from hex_color and is used to specify the background color.
 */
class background_color final : public hex_color {
public:
  explicit background_color(std::string value) : hex_color(std::move(value)) {
  }
};

/**
 * @class foreground_color
 * @brief Represents the foreground color of a component. (aww tag #ag9ycqxx0zp)
 *
 * This class inherits from hex_color and is used to specify the foreground color.
 */
class foreground_color final : public hex_color {
public:
  explicit foreground_color(std::string value) : hex_color(std::move(value)) {
  }
};

} // namespace aww
#endif // AWW_CONFIGURATION_VALUE_OBJECTS_HPP
