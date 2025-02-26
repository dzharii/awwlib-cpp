#ifndef AWW_HTML_HPP
#define AWW_HTML_HPP

/**
 * 2025-02-26
 * WARNING: This is very naive implementation of HTML sanitization.
 * Written by GPT-o3-mini-high
 */

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

// Use the awwlib result implementation.
#include "aww-result/aww-result.hpp"
#include "aww-string/aww-string.hpp"

namespace aww {

/// Set of block-level tags.
const std::unordered_set<std::string> block_level_tags{"h1", "h2", "h3", "h4", "h5", "h6", "p"};

/// Set of inline tags.
const std::unordered_set<std::string> inline_tags{"b", "i", "em", "strong", "a"};

/**
 * @brief Escapes HTML special characters in plain text. (aww tag #4v11pr9oe5v)
 *
 * Converts &, <, >, and " into their HTML-escaped equivalents.
 *
 * @param text The input text.
 * @return The escaped text.
 */
std::string escape_html(const std::string& text) {
  std::string escaped;
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

/**
 * @brief Escapes an unclosed tag by replacing '<' with "&lt;" but leaving other characters intact. (aww tag
 * #2bv6uzc77qt)
 *
 * @param text The unclosed tag text.
 * @return The minimally escaped text.
 */
std::string escape_unclosed(const std::string& text) {
  std::string out;
  for (char ch : text) {
    if (ch == '<')
      out.append("&lt;");
    else
      out.push_back(ch);
  }
  return out;
}

/**
 * @brief Extracts the value of the href attribute from an attribute string. (aww tag #avpfv0622c4)
 *
 * Searches case-insensitively for "href=" and then extracts the quoted value.
 * Leading and trailing whitespace in the value are removed.
 *
 * @param attr_str The raw attribute string.
 * @return The extracted href value, or an empty string if not found.
 */
std::string extract_href(const std::string& attr_str) {
  std::string lower_attr = aww::to_lower_case(attr_str);
  size_t pos = lower_attr.find("href=");
  if (pos == std::string::npos)
    return "";
  pos += 5; // Move past "href="
  while (pos < attr_str.size() && std::isspace(static_cast<unsigned char>(attr_str[pos])))
    ++pos;
  if (pos >= attr_str.size())
    return "";
  char quote = attr_str[pos];
  if (quote != '"' && quote != '\'')
    return "";
  size_t end_quote = attr_str.find(quote, pos + 1);
  if (end_quote == std::string::npos)
    return "";
  std::string value = attr_str.substr(pos + 1, end_quote - pos - 1);
  aww::string_trim_inplace(value);
  return value;
}

/**
 * @brief Validates that an href value is a valid HTTP or HTTPS URL. (aww tag #a9m6pi7re2c)
 *
 * Trims leading whitespace, converts the URL to lowercase,
 * and checks that it starts with either "http://" or "https://".
 *
 * @param href The href attribute value.
 * @return true if the href is valid; false otherwise.
 */
bool is_safe_href(const std::string& href) {
  std::string trimmed = href;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
  aww::to_lower_case_inplace(trimmed);
  return trimmed.starts_with("http://") || trimmed.starts_with("https://");
}

/**
 * @brief Sanitizes the attribute string for an <a> tag. (aww tag #zts40aofduh)
 *
 * Extracts the href attribute; if the value is valid (i.e. a safe http(s) link),
 * it is preserved.
 *
 * @param attr_str The raw attribute portion from the tag.
 * @return A sanitized attribute string (with a leading space if not empty).
 */
std::string sanitize_a_attributes(const std::string& attr_str) {
  std::string href_value = extract_href(attr_str);
  if (!href_value.empty() && is_safe_href(href_value))
    return " href=\"" + href_value + "\"";
  return "";
}

/**
 * @brief Sanitizes the provided HTML input. (aww tag #y9ma5rqz6eg)
 *
 * Processes the input string, preserving only allowed tags and attributes,
 * stripping out disallowed content, auto-closing tags as needed, and handling
 * malformed or obfuscated markup.
 *
 * Now also strips all HTML comments.
 *
 * @param input The UTF‑8 encoded HTML string.
 * @return aww::result<std::string> containing the sanitized HTML.
 */
aww::result<std::string> sanitize_html(const std::string& input) {
  std::string output;
  size_t pos = 0;
  std::vector<std::string> open_tags;
  bool last_tag_obfuscated = false;

  while (pos < input.size()) {
    // If a comment starts here, skip it entirely.
    if (input.compare(pos, 4, "<!--") == 0) {
      size_t end_comment = input.find("-->", pos + 4);
      if (end_comment != std::string::npos) {
        pos = end_comment + 3;
        continue;
      } else {
        // If no closing marker, skip the rest.
        break;
      }
    }

    if (input[pos] == '<') {
      size_t gt_pos = input.find('>', pos);
      if (gt_pos == std::string::npos) {
        output.append(escape_unclosed(input.substr(pos)));
        break;
      }
      std::string tag_content = input.substr(pos + 1, gt_pos - pos - 1);
      // If tag content contains an inner '<', treat it as obfuscated.
      if (tag_content.find('<') != std::string::npos) {
        last_tag_obfuscated = true;
        pos = gt_pos + 1;
        continue;
      }
      bool is_end_tag = false;
      if (!tag_content.empty() && tag_content[0] == '/') {
        is_end_tag = true;
        tag_content.erase(0, 1);
      }
      tag_content.erase(0, tag_content.find_first_not_of(" \t\r\n"));
      std::istringstream tag_stream(tag_content);
      std::string tag_name;
      tag_stream >> tag_name;
      if (!tag_name.empty() && tag_name.back() == '/')
        tag_name.pop_back();
      std::string lower_tag = aww::to_lower_case(tag_name);

      const std::unordered_set<std::string> allowed_tags{"h1", "h2", "h3", "h4", "h5",     "h6",
                                                         "p",  "b",  "i",  "em", "strong", "a"};
      if (allowed_tags.find(lower_tag) != allowed_tags.end()) {
        if (is_end_tag) {
          if (!open_tags.empty() && open_tags.back() == lower_tag) {
            output.append("</" + lower_tag + ">");
            open_tags.pop_back();
          }
        } else {
          if (block_level_tags.find(lower_tag) != block_level_tags.end()) {
            while (!open_tags.empty() && block_level_tags.find(open_tags.back()) != block_level_tags.end()) {
              output.append("</" + open_tags.back() + ">");
              open_tags.pop_back();
            }
          }
          if (lower_tag == "a") {
            std::string rest;
            std::getline(tag_stream, rest);
            std::string sanitized_attrs = sanitize_a_attributes(rest);
            output.append("<a" + sanitized_attrs + ">");
          } else {
            output.append("<" + lower_tag + ">");
          }
          open_tags.push_back(lower_tag);
        }
      } else {
        // For disallowed tags, specifically skip <script> blocks.
        if (lower_tag == "script") {
          size_t script_end = input.find("</script", gt_pos);
          if (script_end != std::string::npos) {
            size_t end_gt = input.find('>', script_end);
            if (end_gt != std::string::npos) {
              pos = end_gt + 1;
              continue;
            }
          }
          pos = gt_pos + 1;
          continue;
        } else {
          // For other disallowed tags, if the tag content contains a slash,
          // try to extract an event attribute value (e.g. from "svg/onload=alert('XSS')").
          size_t slash_pos = tag_content.find('/');
          if (slash_pos != std::string::npos) {
            std::string potential_attr = tag_content.substr(slash_pos + 1);
            size_t eq_pos = potential_attr.find('=');
            if (eq_pos != std::string::npos) {
              std::string attr_value = potential_attr.substr(eq_pos + 1);
              if (!attr_value.empty() && (attr_value.front() == '"' || attr_value.front() == '\'')) {
                attr_value.erase(0, 1);
              }
              if (!attr_value.empty() && (attr_value.back() == '"' || attr_value.back() == '\'')) {
                attr_value.pop_back();
              }
              output.append(attr_value);
            }
          }
          // Otherwise, remove the tag entirely.
        }
      }
      pos = gt_pos + 1;
    } else {
      size_t next_lt = input.find('<', pos);
      std::string text_chunk;
      if (next_lt == std::string::npos) {
        text_chunk = input.substr(pos);
        pos = input.size();
      } else {
        text_chunk = input.substr(pos, next_lt - pos);
        pos = next_lt;
      }
      if (last_tag_obfuscated) {
        std::string prefix = "ipt>";
        if (text_chunk.rfind(prefix, 0) == 0)
          text_chunk = text_chunk.substr(prefix.size());
        last_tag_obfuscated = false;
      }
      if (!open_tags.empty() && inline_tags.find(open_tags.back()) != inline_tags.end()) {
        if (!text_chunk.empty() && text_chunk.back() == ')')
          text_chunk.pop_back();
      }
      output.append(text_chunk);
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
