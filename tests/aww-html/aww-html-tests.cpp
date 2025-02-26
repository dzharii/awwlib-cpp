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
  // With comments stripped entirely, expected output is:
  std::string expected = R"HTML(<p>Hello  World</p>)HTML";
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

// New Test Case 21: Simple HTML Comment Stripping
TEST_CASE("Test Case 21: Simple HTML Comment Stripping") {
  std::string input = R"HTML(<p>Hello <!-- this is a comment -->World</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Hello World</p>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 22: HTML Comment Containing Script
TEST_CASE("Test Case 22: HTML Comment Containing Script") {
  std::string input = R"HTML(<div><!-- <script>alert('XSS');</script> --><p>Safe content</p></div>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Safe content</p>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 23: Multiple HTML Comments in Document
TEST_CASE("Test Case 23: Multiple HTML Comments") {
  std::string input = R"HTML(<!--First comment--><p>Paragraph</p><!--Second comment-->)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Paragraph</p>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 24: Inline Comment Within Text
TEST_CASE("Test Case 24: Inline Comment Within Text") {
  std::string input = R"HTML(<p>Start<!-- comment -->End</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>StartEnd</p>)HTML";
  CHECK(result.value() == expected);
}

// Credit: These tests were created by Michael Ganss (mganss).
// Profile: https://github.com/mganss
// Project: https://github.com/mganss/HtmlSanitizer

TEST_CASE("mganss::HtmlSanitizer test #1: XSS Locator Test") {
  std::string input = R"HTML(<a href="'';!--"<XSS>=&{()}">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="'';!--">=&amp;{()}"></a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #2: Image XSS with quotes and semicolon") {
  std::string input = R"HTML(<IMG SRC="javascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #3: Image XSS without quotes and semicolon") {
  std::string input = R"HTML(<IMG SRC=javascript:alert('XSS')>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #4: Image XSS case-insensitive protocol") {
  std::string input = R"HTML(<IMG SRC=JaVaScRiPt:alert('XSS')>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #5: Image XSS with encoded tab") {
  std::string input = R"HTML(<IMG SRC="jav&#x09;ascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #6: Image XSS with encoded newline") {
  std::string input = R"HTML(<IMG SRC="jav&#x0A;ascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #7: Image XSS with encoded carriage return") {
  std::string input = R"HTML(<IMG SRC="jav&#x0D;ascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #8: Image XSS using grave accents") {
  std::string input = R"HTML(<IMG SRC=`javascript:alert("RSnake says, 'XSS'")`>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #9: Image XSS with numeric entity encoding") {
  std::string input =
      R"HTML(<IMG SRC=&#106;&#97;&#118;&#97;&#115;&#99;&#114;&#105;&#112;&#116;&#58;&#97;&#108;&#101;&#114;&#116;&#40;&#39;&#88;&#83;&#83;&#39;&#41;>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<img>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #10: Iframe XSS") {
  std::string input = R"HTML(<IFRAME SRC="javascript:alert('XSS');"></IFRAME>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML"; // empty output
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #11: Anchor with javascript href") {
  std::string input = R"HTML(<A HREF="javascript:alert(1)">XSS</A>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>XSS</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #12: Script tag removal") {
  std::string input = R"HTML(<script>alert('xss')</script>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML"; // empty output
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #13: Div with dangerous background-image") {
  std::string input = R"HTML(<DIV STYLE="background-image: url(javascript:alert('XSS'))">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<div></div>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #14: Base tag with javascript href") {
  std::string input = R"HTML(<BASE HREF="javascript:alert('XSS');//">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML"; // empty output
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #15: Embed tag removal") {
  std::string input = R"HTML(<EMBED SRC="http://ha.ckers.org/xss.swf" AllowScriptAccess="always"></EMBED>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML"; // empty output
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #16: XML with CDATA removal") {
  std::string input = R"HTML(<XML ID=I><X><C><![CDATA[<IMG SRC="javascript:alert('XSS');">]]></C></X></xml>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML"; // empty output
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #17: Mailto href removal in anchor") {
  std::string input = R"HTML(<a href="mailto:test@example.com">test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #18: Valid HTTP anchor remains") {
  std::string input = R"HTML(<a href="http://example.com">test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="http://example.com">test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #19: Invalid FTP href removed") {
  std::string input = R"HTML(<a href="ftp://example.com">test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #20: CSS expression removal") {
  std::string input = R"HTML(<DIV STYLE="width: expression(alert('foo'));">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<div></div>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #21: Data URI in anchor removed") {
  std::string input = R"HTML(<a href="data:text/html;base64,PHNjcmlwdD5hbGVydCgxKTwvc2NyaXB0Pg==">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a>Test</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #22: HTML Comment Stripping 1") {
  std::string input = R"HTML(<p>Hello <!-- this is a comment -->World</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Hello World</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #23: HTML Comment Stripping 2") {
  std::string input = R"HTML(<div><!-- <script>alert('XSS');</script> --><p>Safe content</p></div>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Safe content</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #24: Multiple HTML Comments") {
  std::string input = R"HTML(<!--First comment--><p>Paragraph</p><!--Second comment-->)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Paragraph</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #25: Inline HTML Comment") {
  std::string input = R"HTML(<p>Start<!-- comment -->End</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>StartEnd</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #26: HTML Comment with Embedded Script") {
  std::string input = R"HTML(<p>Hello <!-- <script>alert('XSS')</script> --> World</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Hello  World</p>)HTML";
  CHECK(result.value() == expected);
}
