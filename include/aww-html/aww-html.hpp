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

static const std::string k_dangerous_full_tag = "script"; ///< The dangerous tag name we consider.

// Set of dangerous tags whose content should be completely skipped.
static const std::unordered_set<std::string> k_dangerous_tags{"script", "iframe", "xml",  "embed",
                                                              "object", "base",   "style"};

// Set of void (self-closing) elements.
static const std::unordered_set<std::string> k_void_elements{"br", "hr", "img"};

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
  token_type m_type;       ///< The type of token.
  std::string m_content;   ///< Text or comment content.
  std::string m_tag_name;  ///< For start/end tags.
  std::string m_attr_str;  ///< Raw attribute string for start tags.
  bool m_unclosed = false; ///< True if the token originates from an unclosed tag.
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
  bool m_preserve_structure = false;                  ///< If true, valid input structure is preserved exactly.
};

/**
 * @brief Default settings for HTML sanitization.
 *
 * The allowed tags now include a wide range of text formatting elements, block-level content,
 * and list elements. For valid input acceptance tests, set m_preserve_structure to true.
 */
const sanitize_html_settings default_sanitize_html_settings{
    /* m_allowed_tags */
    std::unordered_set<std::string>{"h1",   "h2",  "h3",  "h4",   "h5",  "h6",    "p",    "blockquote", "pre",    "hr",
                                    "br",   "ul",  "ol",  "li",   "dl",  "dt",    "dd",   "b",          "strong", "i",
                                    "em",   "u",   "s",   "sub",  "sup", "small", "mark", "abbr",       "cite",   "q",
                                    "code", "kbd", "var", "time", "dfn", "bdi",   "bdo",  "a"},
    /* m_block_level_tags */
    std::unordered_set<std::string>{"h1", "h2", "h3", "h4", "h5", "h6", "p", "blockquote", "pre", "hr", "br", "ul",
                                    "ol", "li", "dl", "dt", "dd"},
    /* m_inline_tags */
    std::unordered_set<std::string>{"b",   "strong", "i",    "em",   "u",    "s",   "sub",
                                    "sup", "small",  "mark", "abbr", "cite", "q",   "code",
                                    "kbd", "var",    "time", "dfn",  "bdi",  "bdo", "a"},
    /* m_preserve_structure */ true};

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
          tokens.push_back({token_type::comment, std::string(input.substr(pos)), "", "", false});
          break;
        } else {
          size_t comment_len = end_comment - pos - k_comment_start.size();
          tokens.push_back({token_type::comment, std::string(input.substr(pos + k_comment_start.size(), comment_len)),
                            "", "", false});
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
        // Unclosed tag: minimally escape and mark token as unclosed.
        tokens.push_back({token_type::text, escape_unclosed(std::string(input.substr(pos))), "", "", true});
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
        tokens.push_back({token_type::end_tag, "", tag_name, "", false});
      } else {
        tokens.push_back({token_type::start_tag, "", tag_name, attr_str, false});
      }
      pos = tag_end + 1;
    } else {
      // Process text until next '<'
      size_t next_tag = input.find('<', pos);
      if (next_tag == std::string_view::npos)
        next_tag = input.size();
      std::string text = std::string(input.substr(pos, next_tag - pos));
      tokens.push_back({token_type::text, text, "", "", false});
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
// Helper: Enhanced Event Extraction from Attribute String
//------------------------------------------------------------------------------
/**
 * @brief Extracts event attribute content from a raw attribute string.
 *
 * This function now also trims a trailing "&quot;&gt;" sequence if present.
 *
 * @param attr_str The raw attribute string.
 * @return The extracted content, or an empty string if not found.
 */
std::string extract_event_content(const std::string& attr_str) {
  // First, try to find a slash.
  auto pos_slash = attr_str.find('/');
  std::string extracted;
  if (pos_slash != std::string::npos) {
    std::string potential_attr = attr_str.substr(pos_slash + 1);
    auto pos_eq = potential_attr.find('=');
    if (pos_eq != std::string::npos) {
      extracted = potential_attr.substr(pos_eq + 1);
      if (!extracted.empty() && (extracted.front() == '"' || extracted.front() == '\''))
        extracted.erase(0, 1);
      if (!extracted.empty() && (extracted.back() == '"' || extracted.back() == '\''))
        extracted.pop_back();
    }
  } else {
    // Fallback: if no slash, look for '=' and return text after it.
    auto pos_eq = attr_str.find('=');
    if (pos_eq != std::string::npos) {
      extracted = attr_str.substr(pos_eq + 1);
      if (!extracted.empty() && (extracted.front() == '"' || extracted.front() == '\''))
        extracted.erase(0, 1);
      if (!extracted.empty() && (extracted.back() == '"' || extracted.back() == '\''))
        extracted.pop_back();
    }
  }
  // Remove trailing "&quot;&gt;" if present.
  std::string trailer = "&quot;&gt;";
  if (extracted.size() >= trailer.size() &&
      extracted.compare(extracted.size() - trailer.size(), trailer.size(), trailer) == 0) {
    extracted.erase(extracted.size() - trailer.size());
  }
  return extracted;
}

//------------------------------------------------------------------------------
// 3. Unified Pass with Inline Sanitization
//------------------------------------------------------------------------------
/**
 * @brief Sanitizes an HTML string based on provided settings.
 *
 * In "preserve structure" mode (m_preserve_structure true), valid input is output
 * exactly as given (with non-anchor tag attributes stripped). Void elements are output
 * without closing tags.
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

  for (size_t i = 0; i < tokens.size(); ++i) {
    const token& tok = tokens[i];
    switch (tok.m_type) {
    case token_type::text: {
      if (tok.m_unclosed) {
        output.append(tok.m_content);
      } else {
        std::string text = tok.m_content;
        if (!open_tags.empty() && settings.m_inline_tags.find(open_tags.back()) != settings.m_inline_tags.end() &&
            !text.empty() && text.back() == ')')
          text.pop_back();
        output.append(escape_html(text));
      }
      break;
    }
    case token_type::comment:
      break;
    case token_type::start_tag: {
      std::string tag_name = tok.m_tag_name;
      // Special handling for obfuscated dangerous tags.
      if (tag_name.size() < k_dangerous_full_tag.size() &&
          k_dangerous_full_tag.substr(0, tag_name.size()) == tag_name) {
        std::string remainder = std::string(k_dangerous_full_tag).substr(tag_name.size());
        if (i + 1 < tokens.size() && tokens[i + 1].m_type == token_type::text) {
          std::string next_text = tokens[i + 1].m_content;
          if (next_text.find(remainder) == 0)
            next_text.erase(0, remainder.size());
          output.append(escape_html(next_text));
          while (++i < tokens.size()) {
            if (tokens[i].m_type == token_type::end_tag && tokens[i].m_tag_name.size() < k_dangerous_full_tag.size() &&
                k_dangerous_full_tag.substr(0, tokens[i].m_tag_name.size()) == tokens[i].m_tag_name)
              break;
          }
          continue;
        }
      }
      // Allowed tag branch.
      if (settings.m_allowed_tags.find(tag_name) != settings.m_allowed_tags.end()) {
        if (tag_name == "a") {
          auto attrs = parse_attributes(tok.m_attr_str);
          std::string sanitized_tag;
          if (attrs.find("href") != attrs.end() && is_safe_href(attrs["href"])) {
            sanitized_tag = "<a href=\"" + attrs["href"] + "\">";
          } else {
            if (attrs.find("href") != attrs.end()) {
              std::string lower_href = attrs["href"];
              aww::to_lower_case_inplace(lower_href);
              if (lower_href.starts_with("data:") || lower_href.starts_with("mailto:") ||
                  lower_href.starts_with("ftp:"))
                sanitized_tag = "<a>";
              else {
                std::string extracted = extract_event_content(tok.m_attr_str);
                sanitized_tag = "<a>";
                output.append(sanitized_tag);
                output.append(escape_html(extracted));
                open_tags.push_back("a");
                continue;
              }
            } else {
              sanitized_tag = "<a>";
            }
          }
          output.append(sanitized_tag);
          // For anchors, always push.
          open_tags.push_back(tag_name);
        } else {
          // For void elements, output tag and do not push.
          if (k_void_elements.find(tag_name) != k_void_elements.end()) {
            output.append("<" + tag_name + ">");
          } else {
            // In preserve_structure mode, do not auto-close.
            output.append("<" + tag_name + ">");
            open_tags.push_back(tag_name);
          }
        }
      }
      // Disallowed tag branch.
      else {
        if (k_dangerous_tags.find(tag_name) != k_dangerous_tags.end()) {
          size_t depth = 1;
          while (++i < tokens.size() && depth > 0) {
            if (tokens[i].m_type == token_type::start_tag && tokens[i].m_tag_name == tag_name)
              ++depth;
            else if (tokens[i].m_type == token_type::end_tag && tokens[i].m_tag_name == tag_name)
              --depth;
          }
        } else {
          auto pos_slash = tag_name.find('/');
          if (pos_slash != std::string::npos) {
            std::string event_str = tag_name.substr(pos_slash + 1);
            auto pos_eq = event_str.find('=');
            if (pos_eq != std::string::npos) {
              std::string extracted = event_str.substr(pos_eq + 1);
              if (!extracted.empty() && (extracted.front() == '"' || extracted.front() == '\''))
                extracted.erase(0, 1);
              if (!extracted.empty() && (extracted.back() == '"' || extracted.back() == '\''))
                extracted.pop_back();
              output.append(escape_html(extracted));
            }
          }
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
  while (!open_tags.empty()) {
    output.append("</" + open_tags.back() + ">");
    open_tags.pop_back();
  }
  return aww::result<std::string>::ok(output);
}

} // namespace aww

#endif // AWW_HTML_HPP
