#ifndef AWW_HTML_HPP
#define AWW_HTML_HPP

/**
 * @file aww_html.hpp
 * @brief Provides HTML sanitization via tokenization, robust attribute parsing, and unified processing.
 *
 * This header tokenizes HTML input, parses attributes with a mini‐parser, and produces sanitized HTML.
 * It implements additional heuristics for dangerous/disallowed tags.
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
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Assume these come from awwlib.
#include "aww-result/aww-result.hpp"
#include "aww-string/aww-string.hpp"

namespace aww {

//------------------------------------------------------------------------------
// Constants for Magic Values
//------------------------------------------------------------------------------
constexpr std::string_view k_comment_start = "<!--";    ///< HTML comment start marker.
constexpr std::string_view k_comment_end = "-->";       ///< HTML comment end marker.
constexpr std::string_view k_cdata_start = "<![CDATA["; ///< CDATA section start.
constexpr std::string_view k_cdata_end = "]]>";         ///< CDATA section end.
constexpr std::string_view k_http_prefix = "http://";   ///< Safe HTTP prefix.
constexpr std::string_view k_https_prefix = "https://"; ///< Safe HTTPS prefix.

// Set of dangerous tags whose content should be completely skipped.
static const std::unordered_set<std::string> k_dangerous_tags{"script", "iframe", "xml",  "embed",
                                                              "object", "base",   "style"};

//------------------------------------------------------------------------------
// Utility: Minimal Escaping for Unclosed Tags
//------------------------------------------------------------------------------
/**
 * @brief Escapes only the '<' character in a string.
 *
 * @param text The input text.
 * @return The minimally escaped string.
 */
std::string escape_unclosed(const std::string& text) {
  std::string out;
  out.reserve(text.size());
  for (char ch : text) {
    if (ch == '<')
      out.append("&lt;");
    else
      out.push_back(ch);
  }
  return out;
}

//------------------------------------------------------------------------------
// Tokenization Types
//------------------------------------------------------------------------------

/**
 * @brief Enumerates token types.
 */
enum class token_type {
  text,      ///< Plain text.
  start_tag, ///< Opening tag.
  end_tag,   ///< Closing tag.
  comment    ///< Comment.
};

/**
 * @brief Represents a token from the HTML input.
 */
struct token {
  token_type m_type;      ///< The type of token.
  std::string m_content;  ///< Text or comment content.
  std::string m_tag_name; ///< For start/end tags.
  std::string m_attr_str; ///< Raw attribute string for start tags.
};

//------------------------------------------------------------------------------
// Sanitization Settings
//------------------------------------------------------------------------------

/**
 * @brief Settings for HTML sanitization.
 */
struct sanitize_html_settings {
  std::unordered_set<std::string> m_allowed_tags;     ///< Allowed tag names.
  std::unordered_set<std::string> m_block_level_tags; ///< Block-level tag names.
  std::unordered_set<std::string> m_inline_tags;      ///< Inline tag names.
};

/**
 * @brief Default settings for HTML sanitization.
 */
const sanitize_html_settings default_sanitize_html_settings{
    /* m_allowed_tags */
    std::unordered_set<std::string>{"h1", "h2", "h3", "h4", "h5", "h6", "p", "b", "i", "em", "strong", "a"},
    /* m_block_level_tags */
    std::unordered_set<std::string>{"h1", "h2", "h3", "h4", "h5", "h6", "p"},
    /* m_inline_tags */
    std::unordered_set<std::string>{"b", "i", "em", "strong", "a"}};

//------------------------------------------------------------------------------
// Utility: HTML Escaping
//------------------------------------------------------------------------------

/**
 * @brief Escapes HTML special characters.
 *
 * @param text The input text.
 * @return The escaped string.
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
    // Handle CDATA sections by skipping them entirely.
    if (input.substr(pos, k_cdata_start.size()) == k_cdata_start) {
      size_t end_cdata = input.find(k_cdata_end, pos + k_cdata_start.size());
      if (end_cdata == std::string_view::npos) {
        pos = input.size();
        continue;
      } else {
        pos = end_cdata + k_cdata_end.size();
        continue;
      }
    }
    if (input[pos] == '<') {
      // Check for comment.
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
        // Unclosed tag: minimally escape.
        tokens.push_back({token_type::text, escape_unclosed(std::string(input.substr(pos))), "", ""});
        break;
      }
      std::string_view tag_content = input.substr(tag_start, tag_end - tag_start);
      // Convert to string for stream parsing.
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
      // Process text until next '<'
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
 * @brief Parses an attribute string into a map of attribute name-value pairs.
 *
 * @param attr_str The raw attribute string.
 * @return A map of attributes (names in lowercase).
 */
std::map<std::string, std::string> parse_attributes(const std::string& attr_str) {
  std::map<std::string, std::string> attrs;
  size_t pos = 0;
  while (pos < attr_str.size()) {
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
      ++pos;
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
 * @brief Determines if an href value is safe.
 *
 * @param href The href string.
 * @return true if safe; false otherwise.
 */
bool is_safe_href(const std::string& href) {
  std::string trimmed = href;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
  aww::to_lower_case_inplace(trimmed);
  return trimmed.starts_with(k_http_prefix) || trimmed.starts_with(k_https_prefix);
}

//------------------------------------------------------------------------------
// Helper: Extract Event Content from Attribute String
//------------------------------------------------------------------------------
/**
 * @brief Extracts event attribute content from a raw attribute string.
 *
 * @param attr_str The raw attribute string.
 * @return The extracted content, or an empty string if not found.
 */
std::string extract_event_content(const std::string& attr_str) {
  auto pos_slash = attr_str.find('/');
  if (pos_slash != std::string::npos) {
    std::string potential_attr = attr_str.substr(pos_slash + 1);
    auto pos_eq = potential_attr.find('=');
    if (pos_eq != std::string::npos) {
      std::string attr_value = potential_attr.substr(pos_eq + 1);
      if (!attr_value.empty() && (attr_value.front() == '"' || attr_value.front() == '\''))
        attr_value.erase(0, 1);
      if (!attr_value.empty() && (attr_value.back() == '"' || attr_value.back() == '\''))
        attr_value.pop_back();
      return attr_value;
    }
  }
  return "";
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
  // For unsafe anchor tags, store extracted inner text.
  std::unordered_map<size_t, std::string> pending_anchor_inner;

  // Process tokens using an index-based loop so that we can skip dangerous tag content.
  for (size_t i = 0; i < tokens.size(); ++i) {
    const token& tok = tokens[i];
    switch (tok.m_type) {
    case token_type::text: {
      std::string text = tok.m_content;
      // If the last open tag is inline, trim trailing ')' (heuristic).
      if (!open_tags.empty() && settings.m_inline_tags.find(open_tags.back()) != settings.m_inline_tags.end() &&
          !text.empty() && text.back() == ')') {
        text.pop_back();
      }
      output.append(escape_html(text));
      break;
    }
    case token_type::comment:
      // Completely skip comments.
      break;
    case token_type::start_tag: {
      std::string tag_name = tok.m_tag_name;
      // If tag is allowed:
      if (settings.m_allowed_tags.find(tag_name) != settings.m_allowed_tags.end()) {
        if (tag_name == "a") {
          // For <a> tags, parse attributes.
          auto attrs = parse_attributes(tok.m_attr_str);
          std::string sanitized_tag;
          if (attrs.find("href") != attrs.end() && is_safe_href(attrs["href"])) {
            sanitized_tag = "<a href=\"" + attrs["href"] + "\">";
          } else {
            // Extract event content from the attribute string.
            std::string extracted = extract_event_content(tok.m_attr_str);
            sanitized_tag = "<a>";
            // Immediately output the extracted inner text.
            output.append(sanitized_tag);
            output.append(escape_html(extracted));
            open_tags.push_back("a");
            // Continue to next token (skip normal anchor processing).
            continue;
          }
          output.append(sanitized_tag);
        } else {
          // For non-anchor allowed tags.
          // Auto-close previous block-level tags if needed.
          if (settings.m_block_level_tags.find(tag_name) != settings.m_block_level_tags.end()) {
            while (!open_tags.empty() &&
                   settings.m_block_level_tags.find(open_tags.back()) != settings.m_block_level_tags.end()) {
              output.append("</" + open_tags.back() + ">");
              open_tags.pop_back();
            }
          }
          output.append("<" + tag_name + ">");
        }
        open_tags.push_back(tag_name);
      }
      // For disallowed tags:
      else {
        // If tag is dangerous, skip all tokens until the matching end tag.
        if (k_dangerous_tags.find(tag_name) != k_dangerous_tags.end()) {
          // Skip tokens until the matching end tag is found.
          size_t depth = 1;
          while (++i < tokens.size() && depth > 0) {
            if (tokens[i].m_type == token_type::start_tag && tokens[i].m_tag_name == tag_name) {
              ++depth;
            } else if (tokens[i].m_type == token_type::end_tag && tokens[i].m_tag_name == tag_name) {
              --depth;
            }
          }
          // Dangerous tag and its contents are skipped.
        } else {
          // For other disallowed tags, try heuristic extraction if a slash is present.
          if (tag_name.find('/') != std::string::npos) {
            std::string extracted = extract_event_content(tok.m_attr_str);
            output.append(escape_html(extracted));
          }
          // Otherwise, ignore the tag entirely.
        }
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
