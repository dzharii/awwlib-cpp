#ifndef AWW_HTML_HPP
#define AWW_HTML_HPP

/**
 * @file aww_html.hpp
 * @brief Provides HTML sanitization via tokenization, robust attribute parsing, and unified processing.
 *
 * This header tokenizes HTML input, parses attributes using a mini‑parser, and produces sanitized HTML.
 * Allowed tag settings are provided through a settings structure.
 *
 * @date 2025-03-01
 */

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

// Assume these come from awwlib.
#include "aww-result/aww-result.hpp"
#include "aww-string/aww-string.hpp"

namespace aww {

//------------------------------------------------------------------------------
// Constants for Magic Values
//------------------------------------------------------------------------------
constexpr std::string_view k_comment_start = "<!--";    ///< Start marker for HTML comments.
constexpr std::string_view k_comment_end = "-->";       ///< End marker for HTML comments.
constexpr std::string_view k_http_prefix = "http://";   ///< Safe HTTP URL prefix.
constexpr std::string_view k_https_prefix = "https://"; ///< Safe HTTPS URL prefix.

//------------------------------------------------------------------------------
// Tokenization Types
//------------------------------------------------------------------------------

/**
 * @brief Enumerates the types of tokens extracted from HTML.
 */
enum class token_type {
  text,      ///< Plain text.
  start_tag, ///< Opening HTML tag.
  end_tag,   ///< Closing HTML tag.
  comment    ///< HTML comment.
};

/**
 * @brief Represents a token from the HTML input.
 */
struct token {
  token_type m_type;      ///< The type of token.
  std::string m_content;  ///< Text content or comment content.
  std::string m_tag_name; ///< Tag name for start/end tags.
  std::string m_attr_str; ///< Raw attribute string for start tags.
};

//------------------------------------------------------------------------------
// Sanitization Settings
//------------------------------------------------------------------------------

/**
 * @brief Configuration for HTML sanitization.
 */
struct sanitize_html_settings {
  std::unordered_set<std::string> m_allowed_tags;     ///< Allowed tag names.
  std::unordered_set<std::string> m_block_level_tags; ///< Tags treated as block-level.
  std::unordered_set<std::string> m_inline_tags;      ///< Tags treated as inline.
};

/**
 * @brief The default settings for HTML sanitization.
 */
const sanitize_html_settings default_sanitize_html_settings{
    /* m_allowed_tags */
    std::unordered_set<std::string>{"h1", "h2", "h3", "h4", "h5", "h6", "p", "b", "i", "em", "strong", "a"},
    /* m_block_level_tags */
    std::unordered_set<std::string>{"h1", "h2", "h3", "h4", "h5", "h6", "p"},
    /* m_inline_tags */
    std::unordered_set<std::string>{"b", "i", "em", "strong", "a"}};

//------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------

/**
 * @brief Escapes HTML special characters in a string.
 *
 * @param text The input text.
 * @return A string with HTML entities escaped.
 */
std::string escape_html(const std::string& text) {
  std::string escaped;
  escaped.reserve(text.size());
  for (char ch : text) {
    switch (ch) {
    case '<':
      escaped.append("&lt;");
      break;
    case '>':
      escaped.append("&gt;");
      break;
    case '&':
      escaped.append("&amp;");
      break;
    case '"':
      escaped.append("&quot;");
      break;
    default:
      escaped.push_back(ch);
      break;
    }
  }
  return escaped;
}

//------------------------------------------------------------------------------
// 1. Tokenization: State Machine Parser
//------------------------------------------------------------------------------

/**
 * @brief Tokenizes HTML input into a vector of tokens.
 *
 * @param input The HTML input as a string_view.
 * @return A vector of tokens.
 */
std::vector<token> tokenize_html(std::string_view input) {
  std::vector<token> tokens;
  size_t pos = 0;
  while (pos < input.size()) {
    if (input[pos] == '<') {
      // Check for HTML comment.
      if (input.substr(pos, k_comment_start.size()) == k_comment_start) {
        size_t end_comment = input.find(k_comment_end, pos + k_comment_start.size());
        if (end_comment == std::string_view::npos) {
          tokens.push_back({token_type::comment, std::string(input.substr(pos)), "", ""});
          break;
        } else {
          size_t comment_len = end_comment - pos - k_comment_start.size();
          tokens.push_back(
              {token_type::comment, std::string(input.substr(pos + k_comment_start.size(), comment_len)), "", ""});
          pos = end_comment + k_comment_end.size();
          continue;
        }
      }
      // Determine if it's an end tag.
      bool is_end_tag = false;
      size_t tag_start = pos + 1;
      if (tag_start < input.size() && input[tag_start] == '/') {
        is_end_tag = true;
        ++tag_start;
      }
      // Find closing '>'.
      size_t tag_end = input.find('>', pos);
      if (tag_end == std::string_view::npos) {
        tokens.push_back({token_type::text, escape_html(std::string(input.substr(pos))), "", ""});
        break;
      }
      std::string_view tag_content = input.substr(tag_start, tag_end - tag_start);
      // Explicitly convert tag_content to std::string for istringstream.
      std::string tag_content_str{tag_content};
      std::istringstream stream(tag_content_str);
      std::string tag_name;
      stream >> tag_name;
      std::string attr_str;
      std::getline(stream, attr_str);
      tag_name = aww::to_lower_case(tag_name);
      if (is_end_tag) {
        tokens.push_back({token_type::end_tag, "", tag_name, ""});
      } else {
        tokens.push_back({token_type::start_tag, "", tag_name, attr_str});
      }
      pos = tag_end + 1;
    } else {
      // Process text until the next '<'.
      size_t next_tag = input.find('<', pos);
      if (next_tag == std::string_view::npos)
        next_tag = input.size();
      std::string text = std::string(input.substr(pos, next_tag - pos));
      tokens.push_back({token_type::text, text, "", ""});
      pos = next_tag;
    }
  }
  return tokens;
}

//------------------------------------------------------------------------------
// 2. Robust Attribute Parsing: Mini-Parser
//------------------------------------------------------------------------------

/**
 * @brief Parses a tag's attribute string into a map of attribute names and values.
 *
 * @param attr_str The raw attribute string.
 * @return A map from attribute names (in lowercase) to their values.
 */
std::map<std::string, std::string> parse_attributes(const std::string& attr_str) {
  std::map<std::string, std::string> attrs;
  size_t pos = 0;
  while (pos < attr_str.size()) {
    // Skip whitespace.
    while (pos < attr_str.size() && std::isspace(static_cast<unsigned char>(attr_str[pos])))
      ++pos;
    if (pos >= attr_str.size())
      break;
    size_t name_start = pos;
    while (pos < attr_str.size() && !std::isspace(static_cast<unsigned char>(attr_str[pos])) && attr_str[pos] != '=')
      ++pos;
    std::string name = attr_str.substr(name_start, pos - name_start);
    name = aww::to_lower_case(name);
    while (pos < attr_str.size() && std::isspace(static_cast<unsigned char>(attr_str[pos])))
      ++pos;
    std::string value;
    if (pos < attr_str.size() && attr_str[pos] == '=') {
      ++pos; // skip '='
      while (pos < attr_str.size() && std::isspace(static_cast<unsigned char>(attr_str[pos])))
        ++pos;
      if (pos < attr_str.size() && (attr_str[pos] == '"' || attr_str[pos] == '\'')) {
        char quote = attr_str[pos];
        ++pos;
        size_t value_start = pos;
        size_t value_end = attr_str.find(quote, pos);
        if (value_end != std::string::npos) {
          value = attr_str.substr(value_start, value_end - value_start);
          pos = value_end + 1;
        } else {
          value = attr_str.substr(value_start);
          pos = attr_str.size();
        }
      } else {
        size_t value_start = pos;
        while (pos < attr_str.size() && !std::isspace(static_cast<unsigned char>(attr_str[pos])))
          ++pos;
        value = attr_str.substr(value_start, pos - value_start);
      }
      aww::string_trim_inplace(value);
    }
    if (!name.empty())
      attrs[name] = value;
  }
  return attrs;
}

//------------------------------------------------------------------------------
// Helper: Safe Href Check
//------------------------------------------------------------------------------

/**
 * @brief Checks whether an href attribute value is safe.
 *
 * @param href The href value.
 * @return true if the href starts with "http://" or "https://", false otherwise.
 */
bool is_safe_href(const std::string& href) {
  std::string trimmed = href;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
  aww::to_lower_case_inplace(trimmed);
  return trimmed.starts_with(k_http_prefix) || trimmed.starts_with(k_https_prefix);
}

//------------------------------------------------------------------------------
// 3. Unified Pass with Inline Sanitization
//------------------------------------------------------------------------------

/**
 * @brief Sanitizes an HTML string based on provided settings.
 *
 * @param input The HTML input.
 * @param settings The sanitization settings.
 * @return A result containing the sanitized HTML.
 */
aww::result<std::string> sanitize_html(const std::string& input,
                                       const sanitize_html_settings& settings = default_sanitize_html_settings) {
  auto tokens = tokenize_html(input);
  std::string output;
  std::vector<std::string> open_tags;

  for (const auto& tok : tokens) {
    switch (tok.m_type) {
    case token_type::text:
      output.append(escape_html(tok.m_content));
      break;
    case token_type::comment:
      // Skip comments.
      break;
    case token_type::start_tag: {
      std::string tag_name = tok.m_tag_name;
      if (settings.m_allowed_tags.find(tag_name) != settings.m_allowed_tags.end()) {
        // Auto-close previous block-level tags if necessary.
        if (settings.m_block_level_tags.find(tag_name) != settings.m_block_level_tags.end()) {
          while (!open_tags.empty() &&
                 settings.m_block_level_tags.find(open_tags.back()) != settings.m_block_level_tags.end()) {
            output.append("</" + open_tags.back() + ">");
            open_tags.pop_back();
          }
        }
        std::string sanitized_tag;
        if (tag_name == "a") {
          auto attrs = parse_attributes(tok.m_attr_str);
          std::string href;
          if (auto it = attrs.find("href"); it != attrs.end() && is_safe_href(it->second))
            href = it->second;
          sanitized_tag = !href.empty() ? "<a href=\"" + href + "\">" : "<a>";
        } else {
          sanitized_tag = "<" + tag_name + ">";
        }
        output.append(sanitized_tag);
        open_tags.push_back(tag_name);
      }
      break;
    }
    case token_type::end_tag: {
      std::string tag_name = tok.m_tag_name;
      if (!open_tags.empty() && open_tags.back() == tag_name) {
        output.append("</" + tag_name + ">");
        open_tags.pop_back();
      }
      break;
    }
    }
  }
  // Auto-close any remaining open tags.
  while (!open_tags.empty()) {
    output.append("</" + open_tags.back() + ">");
    open_tags.pop_back();
  }
  return aww::result<std::string>::ok(output);
}

} // namespace aww

#endif // AWW_HTML_HPP
