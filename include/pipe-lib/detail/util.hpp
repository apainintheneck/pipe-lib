#pragma once

#include <cstdio>
#include <string>
#include <string_view>

//
// General string processing utilities stuffed into the detail namespace.
//
namespace pipe::detail {

template <typename iter>
iter find_next_diff(iter begin, iter end) {
   const auto base_case = begin;
   
   do {
      ++begin;
   } while(begin != end && *begin == *base_case);
   
   return begin;
}

std::string pad_left(std::string&& str, const size_t width) {
   return str.size() < width
      ? std::string(width - str.size(), ' ') + str
      : str;
}

unsigned short count_digits(size_t num) {
   unsigned short count = 0;
   
   while(num > 0) {
      ++count;
      num /= 10;
   }
   
   return count;
}

template <typename T>
std::string pad_left(const T value, const size_t width) {
   return pad_left(std::to_string(value), width);
}

std::string_view skip_whitespace(const std::string_view str) {
   int idx = 0;
   while(idx < str.size() && std::isspace(str[idx]))
      ++idx;
   
   return std::string_view(str.data() + idx, str.size() - idx);
}

size_t line_len(std::string::const_iterator begin, std::string::const_iterator end, const size_t max_len) {
   size_t count = 0;
   while(begin != end and count < max_len) {
      count += (*begin == '\t') ? 8 : 1;
      ++begin;
   }
   return count;
}

size_t line_len_with_end_blank(std::string::const_iterator begin, std::string::const_iterator end, const size_t max_len) {
   size_t count = 0;
   size_t last_blank = 0;
   while(begin != end and count < max_len) {
      count += (*begin == '\t') ? 8 : 1;
      if(std::isspace(*begin)) last_blank = count;
      ++begin;
   }
   return last_blank;
}

namespace char_class {
// These are used to mimic GNU character classes as described here:
//    https://www.gnu.org/software/grep/manual/html_node/Character-Classes-and-Bracket-Expressions.html
constexpr auto blank() { return " \t"; };
constexpr auto cntrl() { return ""; };
constexpr auto digit() { return "0123456789"; };
constexpr auto lower() { return "abcdefghijklmnopqrstuvwxyz"; };
constexpr auto upper() { return "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; };
constexpr auto punct() { return "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; };
constexpr auto space() { return "\t\n\v\f\r "; };
constexpr auto xdigit() { return "0123456789ABCDEFabcdef"; };

}; // namespace char_class

size_t expand_char_class(const std::string& pattern, std::string& dest, size_t idx) {
   auto len = idx;
   while(len < pattern.size() and pattern[len] != ']') {
      ++len;
   }
   
   const auto min_size = strlen("[:rune:]") - 1;
   if(len - idx < min_size and len == pattern.size()) {
      dest.push_back(pattern[idx]);
   } else {
      auto char_class = std::string_view(pattern.data() + idx + 2, len - idx - 3);
      bool is_match = true;
      
      if(char_class == "alnum") {
         dest.append(char_class::digit());
         dest.append(char_class::upper());
         dest.append(char_class::lower());
      } else if(char_class == "alpha") {
         dest.append(char_class::upper());
         dest.append(char_class::lower());
      } else if(char_class == "blank") {
         dest.append(char_class::blank());
      } else if(char_class == "digit") {
         dest.append(char_class::digit());
      } else if(char_class == "graph") {
         dest.append(char_class::digit());
         dest.append(char_class::upper());
         dest.append(char_class::lower());
         dest.append(char_class::punct());
      } else if(char_class == "lower") {
         dest.append(char_class::lower());
      } else if(char_class == "print") {
         dest.append(char_class::digit());
         dest.append(char_class::upper());
         dest.append(char_class::lower());
         dest.append(char_class::punct());
         dest.append(char_class::space());
      } else if(char_class == "punct") {
         dest.append(char_class::punct());
      } else if(char_class == "space") {
         dest.append(char_class::space());
      } else if(char_class == "upper") {
         dest.append(char_class::upper());
      } else if(char_class == "xdigit") {
         dest.append(char_class::xdigit());
      } else {
         is_match = false;
         dest.push_back(pattern[idx]);
      }
      
      if(is_match) idx += len;
   }
   
   return idx;
}

std::string expand_tr_pattern(const std::string& str) {
   std::string expanded_str;
   expanded_str.reserve(str.size());
   
   for(size_t i = 0; i < str.size(); ++i) {
      switch(str[i]) {
         case '\\':
            ++i;
            if(i < str.size())
               expanded_str.push_back(str[i]);
            break;
         case '[':
            i = expand_char_class(str, expanded_str, i);
            break;
         case '-': //Expand range
            if(0 < i - 1 and i + 1 < str.size()) {
               char first = str[i-1];
               char last = str[i+1];
               
               if(first < last) {
                  for(++first; first <= last; ++first) {
                     expanded_str.push_back(first);
                  }
                  ++i;
                  break;
               }
            }
            [[fallthrough]];
         default:
            expanded_str.push_back(str[i]);
            break;
      }
   }
   
   return expanded_str;
}

//
//template <typename iter>
//iter consume_whitespace(iter begin, iter end) {
//   while(begin != end && std::isspace(*begin))
//      ++begin;
//
//   return begin;
//}
//
//template <typename iter>
//iter consume_field(iter begin, iter end) {
//   while(begin != end && !std::isspace(*begin))
//      ++begin;
//
//   return begin;
//}
//
//template <typename iter>
//std::string_view get_field(iter begin, iter end) {
//   return std::string_view(begin, consume_field(begin, end));
//}
//
//std::string_view skip_n_fields(const std::string_view str, size_t n) {
//   auto iter = str.begin();
//   while(iter != str.end() and n > 0) {
//      iter = consume_whitespace(iter, str.end());
//      iter = consume_field(iter, str.end());
//      --n;
//   }
//
//   return str.substr(std::distance(str.begin(), iter));
//}
//
//std::string_view skip_n_chars(const std::string_view str, const size_t n) {
//   return str.substr(n);
//}

} // namespace pipe::detail
