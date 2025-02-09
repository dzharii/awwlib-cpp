#include "aww-configuration-value-objects/aww-configuration-value-objects.hpp"
#include "doctest/doctest.h"

TEST_CASE("HexColorTest: ValidHexColors") {
  // Test valid hex color without alpha
  aww::hex_color color1("#FFFFFF");
  CHECK(color1.red() == 255);
  CHECK(color1.green() == 255);
  CHECK(color1.blue() == 255);
  CHECK(!color1.alpha().has_value());

  // Test valid hex color without alpha
  aww::hex_color color2("#000000");
  CHECK(color2.red() == 0);
  CHECK(color2.green() == 0);
  CHECK(color2.blue() == 0);
  CHECK(!color2.alpha().has_value());

  // Test valid hex color with mixed case
  aww::hex_color color3("#Ff5733");
  CHECK(color3.red() == 255);
  CHECK(color3.green() == 87);
  CHECK(color3.blue() == 51);
  CHECK(!color3.alpha().has_value());

  // Test valid hex color with alpha
  aww::hex_color color4("#12345678");
  CHECK(color4.red() == 18);
  CHECK(color4.green() == 52);
  CHECK(color4.blue() == 86);
  CHECK(color4.alpha().has_value());
  CHECK(color4.alpha().value() == 120);
}

TEST_CASE("HexColorTest: InvalidHexColors") {
  // Test invalid hex color with non-hex characters
  CHECK_THROWS_AS(aww::hex_color("#ZZZZZZ"), std::invalid_argument);

  // Test invalid hex color without '#'
  CHECK_THROWS_AS(aww::hex_color("123456"), std::invalid_argument);

  // Test invalid hex color with incorrect length
  CHECK_THROWS_AS(aww::hex_color("#12345"), std::invalid_argument);
  CHECK_THROWS_AS(aww::hex_color("#123456789"), std::invalid_argument);

  // Test invalid hex color with only '#'
  CHECK_THROWS_AS(aww::hex_color("#"), std::invalid_argument);
}

TEST_CASE("HexColorTest: IsValidHexColor") {
  // Test valid hex colors
  CHECK(aww::hex_color::is_valid("#FFFFFF"));
  CHECK(aww::hex_color::is_valid("#000000"));
  CHECK(aww::hex_color::is_valid("#FF5733"));
  CHECK(aww::hex_color::is_valid("#12345678"));

  // Test invalid hex colors
  CHECK(!aww::hex_color::is_valid("#ZZZZZZ"));
  CHECK(!aww::hex_color::is_valid("123456"));
  CHECK(!aww::hex_color::is_valid("#12345"));
  CHECK(!aww::hex_color::is_valid("#123456789"));
  CHECK(!aww::hex_color::is_valid("#"));
}

TEST_CASE("BackgroundColorTest: ValidBackgroundColor") {
  // Test valid background color
  aww::background_color bgColor("#ABCDEF");
  CHECK(bgColor.red() == 171);
  CHECK(bgColor.green() == 205);
  CHECK(bgColor.blue() == 239);
  CHECK(!bgColor.alpha().has_value());
}

TEST_CASE("ForegroundColorTest: ValidForegroundColor") {
  // Test valid foreground color with alpha
  aww::foreground_color fgColor("#12345678");
  CHECK(fgColor.red() == 18);
  CHECK(fgColor.green() == 52);
  CHECK(fgColor.blue() == 86);
  CHECK(fgColor.alpha().has_value());
  CHECK(fgColor.alpha().value() == 120);
}
