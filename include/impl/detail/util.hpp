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

//bool case_insensitive_cmp(const std::string_view a, const std::string_view b) {
//   auto cmp_upper_char = [](const char a, const char b) {
//      return std::toupper(a) == std::toupper(b);
//   };
//
//   return std::equal(a.begin(), a.end(),
//                     b.begin(), b.end(),
//                     cmp_upper_char);
//}
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
