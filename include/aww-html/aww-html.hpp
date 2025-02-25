#ifndef AWW_HTML_HPP
#define AWW_HTML_HPP

#include <algorithm>
// #include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

// Use the awwlib result implementation.
#include "aww-result/aww-result.hpp"
#include "aww-string/aww-string.hpp"

namespace aww {
/**
 * @brief Escapes HTML special characters in plain text.
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
 * @brief Validates that an href value is a valid HTTP or HTTPS URL.
 *
 * This function trims leading whitespace, converts the URL to lowercase,
 * and checks that it starts with either "http://" or "https://".
 *
 * @param href The href attribute value.
 * @return true if the href is valid; false otherwise.
 */
bool is_safe_href(const std::string& href) {
  std::string trimmed = href;
  // Trim leading whitespace.
  trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
  // Convert to lowercase in place.
  aww::to_lower_case_inplace(trimmed); // (aww tag #igx5i94qp5s)
  return trimmed.starts_with("http://") || trimmed.starts_with("https://");
}

/**
 * @brief Sanitizes the attribute string for an <a> tag.
 *
 * Only the href attribute is allowed. If present and valid (i.e. a safe http(s) link),
 * the attribute is preserved.
 *
 * @param attr_str The raw attribute portion from the tag.
 * @return A sanitized attribute string (with a leading space if not empty).
 */
std::string sanitize_a_attributes(const std::string& attr_str) {
  std::istringstream iss(attr_str);
  std::string token;
  std::string sanitized;
  while (iss >> token) {
    auto pos = token.find('=');
    if (pos != std::string::npos) {
      std::string attr_name = token.substr(0, pos);
      std::string attr_value = token.substr(pos + 1);
      // Remove any surrounding quotes.
      if (!attr_value.empty() && (attr_value.front() == '"' || attr_value.front() == '\'')) {
        attr_value.erase(0, 1);
      }
      if (!attr_value.empty() && (attr_value.back() == '"' || attr_value.back() == '\'')) {
        attr_value.pop_back();
      }
      std::string lower_attr_name = aww::to_lower_case(attr_name);
      if (lower_attr_name == "href" && is_safe_href(attr_value)) {
        sanitized.append(" href=\"" + attr_value + "\"");
      }
    }
  }
  return sanitized;
}

/**
 * @brief Sanitizes the provided HTML input.
 *
 * Processes the input string, preserving only allowed tags and attributes,
 * stripping out disallowed content and handling malformed HTML by auto-closing
 * open tags.
 *
 * @param input The UTF‑8 encoded HTML string.
 * @return aww::result<std::string> containing the sanitized HTML or an error.
 */
aww::result<std::string> sanitize_html(const std::string& input) {
  std::string output;
  size_t pos = 0;
  // Stack to track open allowed tags for auto-closing.
  std::vector<std::string> open_tags;

  // Whitelist of allowed HTML tags.
  const std::unordered_set<std::string> allowed_tags{"h1", "h2", "h3", "h4", "h5",     "h6",
                                                     "p",  "b",  "i",  "em", "strong", "a"};

  while (pos < input.size()) {
    if (input[pos] == '<') {
      size_t gt_pos = input.find('>', pos);
      if (gt_pos == std::string::npos) {
        // Malformed tag: escape the remainder.
        output.append(escape_html(input.substr(pos)));
        break;
      }
      // Extract tag content (without '<' and '>').
      std::string tag_content = input.substr(pos + 1, gt_pos - pos - 1);
      bool is_end_tag = false;
      if (!tag_content.empty() && tag_content[0] == '/') {
        is_end_tag = true;
        tag_content.erase(0, 1);
      }
      // Trim leading whitespace.
      tag_content.erase(0, tag_content.find_first_not_of(" \t\r\n"));
      std::istringstream tag_stream(tag_content);
      std::string tag_name;
      tag_stream >> tag_name;
      // Remove any trailing '/'.
      if (!tag_name.empty() && tag_name.back() == '/') {
        tag_name.pop_back();
      }
      // Convert tag name to lowercase.
      std::string lower_tag = aww::to_lower_case(tag_name);

      if (allowed_tags.find(lower_tag) != allowed_tags.end()) {
        if (is_end_tag) {
          output.append("</" + lower_tag + ">");
          if (!open_tags.empty() && open_tags.back() == lower_tag) {
            open_tags.pop_back();
          }
        } else {
          if (lower_tag == "a") {
            std::string rest;
            std::getline(tag_stream, rest);
            std::string sanitized_attrs = sanitize_a_attributes(rest);
            output.append("<a" + sanitized_attrs + ">");
          } else {
            output.append("<" + lower_tag + ">");
          }
          if (tag_content.find('/') == std::string::npos)
            open_tags.push_back(lower_tag);
        }
      } else {
        // For disallowed tags, special-case <script>.
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
          // Heuristic: if an event-handler attribute is present, extract its value.
          size_t on_pos = tag_content.find("on");
          size_t eq_pos = tag_content.find('=', on_pos);
          if (on_pos != std::string::npos && eq_pos != std::string::npos) {
            std::string attr_value = tag_content.substr(eq_pos + 1);
            if (!attr_value.empty() && (attr_value.front() == '"' || attr_value.front() == '\'')) {
              attr_value.erase(0, 1);
            }
            if (!attr_value.empty() && (attr_value.back() == '"' || attr_value.back() == '\'')) {
              attr_value.pop_back();
            }
            output.append(attr_value);
          }
        }
      }
      pos = gt_pos + 1;
    } else {
      // Copy plain text.
      size_t next_lt = input.find('<', pos);
      if (next_lt == std::string::npos) {
        output.append(input.substr(pos));
        pos = input.size();
      } else {
        output.append(input.substr(pos, next_lt - pos));
        pos = next_lt;
      }
    }
  }
  // Auto-close any open tags.
  while (!open_tags.empty()) {
    output.append("</" + open_tags.back() + ">");
    open_tags.pop_back();
  }
  return aww::result<std::string>::ok(output);
}

} // namespace aww
#endif // AWW_HTML_HPP
