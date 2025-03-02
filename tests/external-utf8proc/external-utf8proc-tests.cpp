// external-utf8proc-tests.cpp
// Acceptance tests for the utf8proc library, using the doctest framework.

#include "doctest/doctest.h" // make sure doctest is in your include path
#include "utf8proc.h"        // our utf8proc header from external/utf8proc-2.10.0

#include <cstdlib>
#include <cstring>
#include <string>

// -----------------------------------------------------------------------------
// Test case: Codepoint conversion
// -----------------------------------------------------------------------------
TEST_CASE("UTF8proc: Individual Codepoint Conversion") {
  SUBCASE("ASCII conversion: 'A' to 'a' and vice versa") {
    utf8proc_int32_t A = 'A';
    utf8proc_int32_t a_expected = 'a';
    utf8proc_int32_t a_result = utf8proc_tolower(A);
    CHECK(a_result == a_expected);

    utf8proc_int32_t a_val = 'a';
    utf8proc_int32_t A_expected = 'A';
    utf8proc_int32_t A_result = utf8proc_toupper(a_val);
    CHECK(A_result == A_expected);
  }

  SUBCASE("Non-ASCII conversion: 'Á' (U+00C1) to 'á' (U+00E1)") {
    utf8proc_int32_t Aacute = 0x00C1;          // 'Á'
    utf8proc_int32_t aacute_expected = 0x00E1; // 'á'
    utf8proc_int32_t aacute_result = utf8proc_tolower(Aacute);
    CHECK(aacute_result == aacute_expected);

    utf8proc_int32_t a_val = 0x00E1;      // 'á'
    utf8proc_int32_t A_expected = 0x00C1; // 'Á'
    utf8proc_int32_t A_result = utf8proc_toupper(a_val);
    CHECK(A_result == A_expected);
  }

  SUBCASE("No conversion: digits remain unchanged") {
    utf8proc_int32_t digit = '7';
    CHECK(utf8proc_tolower(digit) == '7');
    CHECK(utf8proc_toupper(digit) == '7');
  }
}

// -----------------------------------------------------------------------------
// Test case: UTF-8 string mapping (case folding)
// -----------------------------------------------------------------------------
TEST_CASE("UTF8proc: Full String Mapping via utf8proc_map") {
  SUBCASE("Case folding converts uppercase letters to lowercase") {
    // Example input (UTF-8 literal). In C++20, u8 literals have type const char8_t*
    const char8_t* input = u8"Neacșu";
    utf8proc_uint8_t* mapped = nullptr;

    // Use UTF8PROC_NULLTERM so the input is treated as null-terminated,
    // and UTF8PROC_CASEFOLD for a case-insensitive transformation.
    utf8proc_ssize_t result =
        utf8proc_map(reinterpret_cast<const utf8proc_uint8_t*>(input),
                     -1, // process until null terminator
                     &mapped, static_cast<utf8proc_option_t>(UTF8PROC_NULLTERM | UTF8PROC_CASEFOLD));
    INFO("utf8proc_map error: " << (result < 0 ? utf8proc_errmsg(result) : "none"));
    CHECK(result >= 0);

    // Create a std::string from the mapped result.
    std::string mappedStr(reinterpret_cast<char*>(mapped));
    // As a simple check, ensure that no uppercase ASCII letters remain.
    for (char c : mappedStr) {
      if (c >= 'A' && c <= 'Z') {
        FAIL_CHECK("Found uppercase ASCII letter: " << c);
      }
    }
    free(mapped);
  }
}

// -----------------------------------------------------------------------------
// Test case: Multi-Language Mapping (Cyrillic and Japanese)
// -----------------------------------------------------------------------------
//
// For the Cyrillic test we encode the strings as UTF-8 bytes via hex escapes.
// "ДМИТРО" (uppercase) should convert to "дмитро" (lowercase).
//   "Д" = 0xD0 0x94, "М" = 0xD0 0x9C, "И" = 0xD0 0x98,
//   "Т" = 0xD0 0xA2, "Р" = 0xD0 0xA0, "О" = 0xD0 0x9E
//
//   "д" = 0xD0 0xB4, "м" = 0xD0 0xBC, "и" = 0xD0 0xB8,
//   "т" = 0xD1 0x82, "р" = 0xD1 0x80, "о" = 0xD0 0xBE
TEST_CASE("UTF8proc: Multi-Language Mapping") {
  SUBCASE("Cyrillic conversion: 'ДМИТРО' to 'дмитро'") {
    // Uppercase "ДМИТРО" in UTF-8 encoded as hexadecimal escapes:
    const char input_cyrillic[] = "\xD0\x94\xD0\x9C\xD0\x98\xD0\xA2\xD0\xA0\xD0\x9E";
    // Expected lowercase "дмитро":
    const char expected_lower_cyrillic[] = "\xD0\xB4\xD0\xBC\xD0\xB8\xD1\x82\xD1\x80\xD0\xBE";

    utf8proc_uint8_t* mapped = nullptr;
    utf8proc_ssize_t result =
        utf8proc_map(reinterpret_cast<const utf8proc_uint8_t*>(input_cyrillic),
                     -1, // process until null terminator
                     &mapped, static_cast<utf8proc_option_t>(UTF8PROC_NULLTERM | UTF8PROC_CASEFOLD));
    INFO("utf8proc_map error: " << (result < 0 ? utf8proc_errmsg(result) : "none"));
    CHECK(result >= 0);

    std::string mappedStr(reinterpret_cast<char*>(mapped));
    CHECK(mappedStr == std::string(expected_lower_cyrillic));
    free(mapped);
  }

  SUBCASE("Japanese text remains unchanged: '日本語'") {
    const char8_t* input = u8"日本語";
    utf8proc_uint8_t* mapped = nullptr;
    utf8proc_ssize_t result =
        utf8proc_map(reinterpret_cast<const utf8proc_uint8_t*>(input),
                     -1, // process until null terminator
                     &mapped, static_cast<utf8proc_option_t>(UTF8PROC_NULLTERM | UTF8PROC_CASEFOLD));
    INFO("utf8proc_map error: " << (result < 0 ? utf8proc_errmsg(result) : "none"));
    CHECK(result >= 0);

    std::string mappedStr(reinterpret_cast<char*>(mapped));
    // Japanese text has no case, so the output should be identical.
    CHECK(mappedStr == std::string(reinterpret_cast<const char*>(u8"日本語")));
    free(mapped);
  }
}

// -----------------------------------------------------------------------------
// Test case: Encode/decode round-trip for individual codepoints
// -----------------------------------------------------------------------------
TEST_CASE("UTF8proc: Encode and Iterate (Decode) are Inverses") {
  // Test a few sample codepoints (ASCII, accented letter, Greek letter, and an emoji)
  utf8proc_int32_t codepoints[] = {'A', 0x00C1, 0x0391, 0x1F600}; // 'A', 'Á', Greek Alpha, 😀
  for (utf8proc_int32_t cp : codepoints) {
    utf8proc_uint8_t buffer[5] = {0}; // up to 4 bytes + null terminator
    utf8proc_ssize_t enc = utf8proc_encode_char(cp, buffer);
    CHECK(enc > 0);

    utf8proc_int32_t decoded = -1;
    utf8proc_ssize_t iter = utf8proc_iterate(buffer, enc, &decoded);
    INFO("Encoding codepoint: " << cp);
    CHECK(iter == enc);
    CHECK(decoded == cp);
  }
}

// -----------------------------------------------------------------------------
// Test case: Unicode property query
// -----------------------------------------------------------------------------
TEST_CASE("UTF8proc: Unicode Property Query") {
  SUBCASE("Category for 'A' is Uppercase Letter") {
    utf8proc_int32_t cp = 'A';
    utf8proc_category_t cat = utf8proc_category(cp);
    CHECK(cat == UTF8PROC_CATEGORY_LU);
  }
}
