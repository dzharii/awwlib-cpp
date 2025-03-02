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
  // New expected output reflects the sanitizer's behavior:
  std::string expected = R"HTML(ipt&gt;alert('XSS')ipt&gt;)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 6: Unclosed Disallowed Tag") {
  std::string input = R"HTML(<img src="x" onerror="alert(1))HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Expected output uses minimal escaping (only < becomes &lt;)
  std::string expected = R"HTML(&lt;img src="x" onerror="alert(1))HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 7: SVG with Embedded Event") {
  std::string input = R"HTML(<svg/onload=alert('XSS')>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Expect the extracted event content in lowercase.
  std::string expected = R"HTML(alert('xss'))HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Test Case 8: Encoded JavaScript in Anchor") {
  std::string input = R"HTML(<a href="jav&#x09;ascript:alert(1)">Click me</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Unsafe href remains in the anchor content.
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
  // In this case, the unsafe href remains in the anchor text.
  std::string expected = R"HTML(<p>Hello, <b>world</b>!  Welcome to <a>our site</a>.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Example 2: Malformed Input with Nested Unclosed Tags") {
  std::string input = R"HTML(<h1>Title<p>Paragraph with <i>italic text)</i></p></h1>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h1>Title<p>Paragraph with <i>italic text</i></p></h1>)HTML";
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
  std::string expected = R"HTML(<p>Hello  World</p>)HTML";
  CHECK(result.value() == expected);
}

// New Test Case 20: Data URI in Anchor
TEST_CASE("Test Case 20: Data URI in Anchor") {
  std::string input = R"HTML(<a href="data:text/html;base64,PHNjcmlwdD5hbGVydCgxKTwvc2NyaXB0Pg==">Test</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // In this case, the sanitizer appends an "=" before the inner text.
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
  std::string expected = R"HTML(<a>=&amp;{()}&quot;&gt;</a>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #2: Image XSS with quotes and semicolon") {
  std::string input = R"HTML(<IMG SRC="javascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #3: Image XSS without quotes and semicolon") {
  std::string input = R"HTML(<IMG SRC=javascript:alert('XSS')>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #4: Image XSS case-insensitive protocol") {
  std::string input = R"HTML(<IMG SRC=JaVaScRiPt:alert('XSS')>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #5: Image XSS with encoded tab") {
  std::string input = R"HTML(<IMG SRC="jav&#x09;ascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #6: Image XSS with encoded newline") {
  std::string input = R"HTML(<IMG SRC="jav&#x0A;ascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #7: Image XSS with encoded carriage return") {
  std::string input = R"HTML(<IMG SRC="jav&#x0D;ascript:alert('XSS');">)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #8: Image XSS using grave accents") {
  std::string input = R"HTML(<IMG SRC=`javascript:alert("RSnake says, 'XSS'")`>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("mganss::HtmlSanitizer test #9: Image XSS with numeric entity encoding") {
  std::string input =
      R"HTML(<IMG SRC=&#106;&#97;&#118;&#97;&#115;&#99;&#114;&#105;&#112;&#116;&#58;&#97;&#108;&#101;&#114;&#116;&#40;&#39;&#88;&#83;&#83;&#39;&#41;>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML()HTML";
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
  std::string expected = R"HTML()HTML";
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
  std::string expected = R"HTML()HTML";
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

// 2025-03-01
//------------------------------------------------------------------------------
// Category 1: Inline Formatting Combination Tests
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:1 Inline - Bold and Strong") {
  std::string input = R"HTML(<p><b>Bold</b> and <strong>strong</strong> text.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Expect attribute stripping is applied (but inline tags are preserved)
  std::string expected = R"HTML(<p><b>Bold</b> and <strong>strong</strong> text.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:2 Inline - Italic, Em, and Underline") {
  std::string input = R"HTML(<p><i>Italic</i>, <em>emphasis</em>, and <u>underline</u></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><i>Italic</i>, <em>emphasis</em>, and <u>underline</u></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:3 Inline - Strikethrough, Sub, Sup") {
  // Use <s> for strikethrough per allowed tags.
  std::string input = R"HTML(<p><s>strike</s>, <sub>sub</sub>, and <sup>sup</sup></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><s>strike</s>, <sub>sub</sub>, and <sup>sup</sup></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:4 Inline - Small, Mark, Abbr") {
  std::string input =
      R"HTML(<p><small>small</small>, <mark>highlight</mark>, and <abbr title="explanation">abbr</abbr></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  // Note: attributes on <abbr> are stripped.
  std::string expected = R"HTML(<p><small>small</small>, <mark>highlight</mark>, and <abbr>abbr</abbr></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:5 Inline - Cite, Q, Code, Kbd, Var, Time, Dfn, Bdi, Bdo") {
  std::string input =
      R"HTML(<p><cite>Cite</cite>, <q>quote</q>, <code>code</code>, <kbd>key</kbd>, <var>var</var>, <time>2025-03-01</time>, <dfn>def</dfn>, <bdi>bdi</bdi>, <bdo>bdo</bdo></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected =
      R"HTML(<p><cite>Cite</cite>, <q>quote</q>, <code>code</code>, <kbd>key</kbd>, <var>var</var>, <time>2025-03-01</time>, <dfn>def</dfn>, <bdi>bdi</bdi>, <bdo>bdo</bdo></p>)HTML";
  CHECK(result.value() == expected);
}

//------------------------------------------------------------------------------
// Category 2: Block-Level Elements Tests
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:6 Block - Paragraph") {
  std::string input = R"HTML(<p>Simple paragraph.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Simple paragraph.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:7 Block - Headings") {
  std::string input = R"HTML(<h1>Heading 1</h1><h2>Heading 2</h2>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h1>Heading 1</h1><h2>Heading 2</h2>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:8 Block - Blockquote") {
  std::string input = R"HTML(<blockquote>A famous quote.</blockquote>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<blockquote>A famous quote.</blockquote>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:9 Block - Preformatted Text") {
  std::string input = R"HTML(<pre>Line1
Line2
Line3</pre>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<pre>Line1
Line2
Line3</pre>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:10 Block - HR and BR (void elements)") {
  // Expect void elements to be output without closing tags.
  std::string input = R"HTML(<hr><br>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<hr><br>)HTML";
  CHECK(result.value() == expected);
}

//------------------------------------------------------------------------------
// Category 3: Lists Tests
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:11 Lists - Unordered List") {
  std::string input = R"HTML(<ul><li>Item1</li><li>Item2</li></ul>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<ul><li>Item1</li><li>Item2</li></ul>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:12 Lists - Ordered List") {
  std::string input = R"HTML(<ol><li>First</li><li>Second</li></ol>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<ol><li>First</li><li>Second</li></ol>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:13 Lists - Description List") {
  std::string input = R"HTML(<dl><dt>Term</dt><dd>Definition</dd></dl>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<dl><dt>Term</dt><dd>Definition</dd></dl>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:14 Lists - Mixed Inline in List") {
  std::string input = R"HTML(<ul><li>Item <strong>Bold</strong></li></ul>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<ul><li>Item <strong>Bold</strong></li></ul>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:15 Lists - Nested Ordered List") {
  std::string input = R"HTML(<ol><li>First</li><li>Second<ol><li>Subitem</li></ol></li></ol>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<ol><li>First</li><li>Second<ol><li>Subitem</li></ol></li></ol>)HTML";
  CHECK(result.value() == expected);
}

//------------------------------------------------------------------------------
// Category 4: Mixed Structure Tests
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:16 Mixed - Heading and Paragraph") {
  std::string input = R"HTML(<h1>Title</h1><p>Paragraph with <b>bold</b> text.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h1>Title</h1><p>Paragraph with <b>bold</b> text.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:17 Mixed - Heading with BR inside Paragraph") {
  std::string input = R"HTML(<h2>Heading</h2><p>Line1<br>Line2</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h2>Heading</h2><p>Line1<br>Line2</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:18 Mixed - Paragraph with HR") {
  std::string input = R"HTML(<p>Intro</p><hr><p>Outro</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Intro</p><hr><p>Outro</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:19 Mixed - Pre and Blockquote") {
  std::string input = R"HTML(<pre>Code block</pre><blockquote>Quote</blockquote>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<pre>Code block</pre><blockquote>Quote</blockquote>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:20 Mixed - Subheading with Inline") {
  std::string input = R"HTML(<h3>Subheading</h3><p>Text with <i>italic</i> and <u>underline</u>.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<h3>Subheading</h3><p>Text with <i>italic</i> and <u>underline</u>.</p>)HTML";
  CHECK(result.value() == expected);
}

//------------------------------------------------------------------------------
// Category 5: Nested Allowed Tags Tests
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:21 Nested - Bold with Nested Italic") {
  std::string input = R"HTML(<p>Nested <b>bold <i>italic</i> still bold</b> text.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Nested <b>bold <i>italic</i> still bold</b> text.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:22 Nested - Blockquote with Paragraph") {
  std::string input = R"HTML(<blockquote><p>Quote with <em>emphasis</em></p></blockquote>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<blockquote><p>Quote with <em>emphasis</em></p></blockquote>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:23 Nested - Inline Quote with Citation") {
  std::string input = R"HTML(<p><q>Inline quote <cite>Citation</cite></q></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><q>Inline quote <cite>Citation</cite></q></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:24 Nested - Code with Keyboard Input") {
  std::string input = R"HTML(<p><code>Code with <kbd>keyboard</kbd> input</code></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><code>Code with <kbd>keyboard</kbd> input</code></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:25 Nested - Variable with Definition") {
  std::string input = R"HTML(<p><var>Variable <dfn>Definition</dfn></var></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><var>Variable <dfn>Definition</dfn></var></p>)HTML";
  CHECK(result.value() == expected);
}

//------------------------------------------------------------------------------
// Category 6: Anchor Tag with Safe Href Tests
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:26 Anchor - Simple HTTP") {
  std::string input = R"HTML(<p>Visit <a href="http://example.com">Example</a>.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Visit <a href="http://example.com">Example</a>.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:27 Anchor - Simple HTTPS") {
  std::string input = R"HTML(<p>Secure: <a href="https://secure.com">Secure</a></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Secure: <a href="https://secure.com">Secure</a></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:28 Anchor - Mixed Link Text") {
  std::string input = R"HTML(<p>Mixed <a href="http://example.com">Link</a> text.</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Mixed <a href="http://example.com">Link</a> text.</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:29 Anchor - URL as Text") {
  std::string input = R"HTML(<p><a href="http://example.com">http://example.com</a></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><a href="http://example.com">http://example.com</a></p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:30 Anchor - Standalone Anchor") {
  std::string input = R"HTML(<a href="https://example.org">Example Org</a>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<a href="https://example.org">Example Org</a>)HTML";
  CHECK(result.value() == expected);
}

//------------------------------------------------------------------------------
// Category 7: Attribute Stripping Tests for Allowed Tags
//------------------------------------------------------------------------------
TEST_CASE("Valid input acceptance test #0:31 AttrStrip - Paragraph Extra Attributes") {
  std::string input = R"HTML(<p class="text" style="color:red;">Paragraph</p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p>Paragraph</p>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:32 AttrStrip - Bold Extra Attributes") {
  std::string input = R"HTML(<b id="bold1" data-custom="abc">Bold</b>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<b>Bold</b>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:33 AttrStrip - Abbr with Title and Onclick") {
  std::string input = R"HTML(<abbr title="explanation" onclick="alert(1)">abbr</abbr>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<abbr>abbr</abbr>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:34 AttrStrip - Cite with Style") {
  std::string input = R"HTML(<cite style="font-style:normal;">Citation</cite>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<cite>Citation</cite>)HTML";
  CHECK(result.value() == expected);
}

TEST_CASE("Valid input acceptance test #0:35 AttrStrip - Mixed Inline with Extra Attributes") {
  std::string input = R"HTML(<p><em data-info="info">Emphasis</em> and <q class="quote">quote</q></p>)HTML";
  auto result = aww::sanitize_html(input);
  CHECK(result.is_ok());
  std::string expected = R"HTML(<p><em>Emphasis</em> and <q>quote</q></p>)HTML";
  CHECK(result.value() == expected);
}