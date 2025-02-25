#include "aww-html/aww-html.hpp"
#include "doctest/doctest.h"

TEST_CASE("Test Case 1: Valid HTML with Allowed Tags") {
  std::string input =
      R"HTML(<h1>Welcome</h1><p>This is a <b>test</b> paragraph with an <a href="http://example.com">example link</a>.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Expected output exactly matches the valid input.
  std::string expected =
      R"HTML(<h1>Welcome</h1><p>This is a <b>test</b> paragraph with an <a href="http://example.com">example link</a>.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 2: Disallowed Tags and Attributes") {
  std::string input = R"HTML(<h1 style="color:red;">Header</h1><script>alert('XSS');</script>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h1>Header</h1>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 3: Anchor Tag with Disallowed Attribute") {
  std::string input = R"HTML(<a href="http://example.com" onclick="alert('XSS')">Click me</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="http://example.com">Click me</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 4: Malformed HTML with Unclosed Tags") {
  std::string input = R"HTML(<p>Paragraph <b>Bold text <i>Italic without closing)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Paragraph <b>Bold text <i>Italic without closing</i></b></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 5: XSS Payload with Obfuscated Tag") {
  std::string input = R"HTML(<scr<script>ipt>alert('XSS')</scr<script>ipt>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(alert('XSS'))HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 6: Unclosed Disallowed Tag") {
  std::string input = R"HTML(<img src="x" onerror="alert(1))HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(&lt;img src="x" onerror="alert(1))HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 7: SVG with Embedded Event") {
  std::string input = R"HTML(<svg/onload=alert('XSS')>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(alert('XSS'))HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 8: Encoded JavaScript in Anchor") {
  std::string input = R"HTML(<a href="jav&#x09;ascript:alert(1)">Click me</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Click me</a>)HTML";
  CHECK(result.value() == expected);
}

// --- Extended Test Cases for Anchor Href Validation ---

TEST_CASE("Test Case 9: Valid HTTP Anchor") {
  std::string input = R"HTML(<a href="http://example.com">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="http://example.com">Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 10: Valid HTTPS Anchor") {
  std::string input = R"HTML(<a href="https://example.com">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="https://example.com">Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 11: Invalid FTP Anchor") {
  std::string input = R"HTML(<a href="ftp://example.com">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 12: Relative URL Anchor") {
  std::string input = R"HTML(<a href="/local/path">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 13: Mailto Anchor") {
  std::string input = R"HTML(<a href="mailto:someone@example.com">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 14: Anchor with Leading Whitespace in URL") {
  std::string input = R"HTML(<a href="   http://example.com">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="http://example.com">Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Example 1: Mixed Valid and Invalid Content") {
  std::string input =
      R"HTML(<p>Hello, <b>world</b>! <img src="invalid" onerror="alert(1)"> Welcome to <a href="javascript:alert(1)">our site</a>.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Hello, <b>world</b>!  Welcome to <a>our site</a>.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Example 2: Malformed Input with Nested Unclosed Tags") {
  std::string input = R"HTML(<h1>Title<p>Paragraph with <i>italic text)</i></p></h1>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h1>Title</h1><p>Paragraph with <i>italic text</i></p>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 15: Numeric Character Reference in Anchor
TEST_CASE("Test Case 15: Numeric Character Reference in Anchor") {
  std::string input =
      R"HTML(<a href="&#x6A;&#x61;&#x76;&#x61;&#x73;&#x63;&#x72;&#x69;&#x70;&#x74;:alert(1)">Click me</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Click me</a>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 16: Mixed Case and Whitespace in Href
TEST_CASE("Test Case 16: Mixed Case and Whitespace in Href") {
  std::string input = R"HTML(<a href="   HTTP://Example.com  ">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="HTTP://Example.com">Test</a>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 17: Inline Event Handler in Allowed Tag
TEST_CASE("Test Case 17: Inline Event Handler in Allowed Tag") {
  std::string input = R"HTML(<h1 onclick="alert(1)">Header</h1>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h1>Header</h1>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 18: Mixed Case <A> Tag with JavaScript Scheme
TEST_CASE("Test Case 18: Mixed Case <A> Tag with JavaScript Scheme") {
  std::string input = R"HTML(<A HREF="JaVaScRiPt:alert(1)">Test</A>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Test</a>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 19: HTML Comment with Embedded Script
TEST_CASE("Test Case 19: HTML Comment with Embedded Script") {
  std::string input = R"HTML(<p>Hello <!-- <script>alert('XSS')</script> --> World</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Comments are not preserved; the sanitizer strips HTML comments.
  std::string expected = R"HTML(<p>Hello alert('XSS') World</p>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 20: Data URI in Anchor
TEST_CASE("Test Case 20: Data URI in Anchor") {
  std::string input = R"HTML(<a href="data:text/html;base64,PHNjcmlwdD5hbGVydCgxKTwvc2NyaXB0Pg==">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Test</a>)HTML";
  CHECK(result.value() == expected);
}
