#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cctype>
#include <string_view>
#include <regex>
#include <map>

#include "option.hpp"
#include "detail/util.hpp"
#include "output.hpp"

//
// The base class for all the following pipe classes.
//
namespace pipe {

class Pipe {
   friend class Builder;
public:
   using StringCmp = std::function<bool(const std::string_view, const std::string_view)>;
   
   ~Pipe() = default;
   
   /*
    
    Output Operators
    
    */
   
   // Overwrites File with piped lines.
   void operator>(const File& file);
   // Appends piped lines to File.
   void operator>>(const File& file);
   // Overwrites string with piped lines.
   void operator>(std::string& str);
   // Appends piped lines to string.
   void operator>>(std::string& str);
   // Append to ostream.
   void operator|(std::ostream& os);
   // Pipe to Tee (append or write options specified in Tee class).
   template<typename tee_option>
   void operator|(Tee<tee_option>& tee);
   
   /*
    
    Pipe Filters
    
    */
   
   //
   // Grep
   //
   
   // Defaults to basic grep.
   // Option::i - Ignore case
   // Option::E - egrep (extended grep)
   template <typename ...Options>
   Pipe grep(const std::string& pattern);
   
   //
   // Head
   //
   
   // Defaults to retaining the top 10 lines.
   Pipe head();
   // Option::n - Retains the top x lines. [Base case]
   template <typename option>
   Pipe head(const size_t count);
   // Option::c - Retains the top x bytes.
   template <>
   Pipe head<opt::c>(const size_t count);
   
   //
   // Sort
   //
   
   // Default lexical sort. [Base case]
   template <typename ...Options>
   Pipe sort();
   // Option::m - Merge two selected ranges by comparator.
   template <typename ...Options>
   Pipe sort(const Pipe& other);
   
   //
   // Tail
   //
   
   // Defaults to retaining the bottom 10 lines.
   Pipe tail();
   // Option::n - Retains the bottom x lines. [Base case]
   template <typename option>
   Pipe tail(const size_t count);
   // Option::c - Retains the bottom x bytes.
   template <>
   Pipe tail<opt::c>(const size_t count);
   
   //
   // Tr
   //
   
   // Defaults to translating characters from pattern1 to pattern2
   // based upon their position.
   Pipe tr(const std::string& pattern1, const std::string& pattern2);
   // Covers the one argument uses of the tr cmd.
   // Requires Option::d or Option::s with optional Option::c
   // Option::d - Deletes matching characters
   // Option::s - Squeezes adjacent matching characters (like unique but only for matching chars).
   // Option::c - Use the compliment of the given pattern.
   template <typename ...Options>
   Pipe tr(const std::string& pattern);
   
   //
   // Uniq
   //
   
   // Defaults to removing adjacent duplicate lines. [Base case]
   template <typename option>
   Pipe uniq();
   // Option::c - Prefix a count of the frequency of each line.
   template <>
   Pipe uniq<opt::c>();
   // Option::d - Only retain duplicate lines.
   template <>
   Pipe uniq<opt::d>();
   // Option::d - Only retain unique lines.
   template <>
   Pipe uniq<opt::u>();
   
   //
   // WC - Word Count
   //
   
   // Takes options c, l, m and w.
   // Option::c - Byte count
   // Option::m - Char count
   // Option::l - Line count
   // Option::w - Word count
   template <typename ...Options>
   Pipe wc();
   
private:
   Pipe() = default;
   
   //
   // Init
   //
   
   // Parse lines of text from input stream and append them to lines vector.
   void append(std::istream& in);
   
   //
   // Helpers
   //
   
   // Number all lines in lines vector.
   void number_lines(const bool skip_blank);
   // Return total number of chars in all lines.
   size_t char_count() const;
   // Merge two sorted Pipes.
   static std::vector<std::string> merge(const Pipe& p1, const Pipe& p2, StringCmp cmp);
   // Get the comparator needed to sort the lines.
   template <typename ...Options>
   static StringCmp sort_cmp();

   //
   // Output
   //
   void pipe_to(std::ostream& os);
   
   //
   // Variables
   //
   std::vector<std::string> lines;
};

/*
 
 Output Operators
 
 */

void Pipe::operator>(const File& file) {
   std::ofstream outfile(file._filename);
   pipe_to(outfile);
}

void Pipe::operator>>(const File& file) {
   std::ofstream outfile(file._filename, std::ios::out | std::ios::app);
   pipe_to(outfile);
}

void Pipe::operator>(std::string& str) {
   str.clear();
   str.reserve(char_count());
   for(const auto& line : lines)
      str += line;
}

void Pipe::operator>>(std::string& str) {
   str.reserve(char_count() + str.length());
   for(const auto& line : lines)
      str += line;
}

void Pipe::operator|(std::ostream& os) {
   pipe_to(os);
}

template<typename tee_option>
void Pipe::operator|(Tee<tee_option>& tee) {
   for(const auto& line : lines) {
      tee << line;
      tee << '\n';
   }
}

/*
 
 Pipe Filters
 
 */

//
// Grep
//

template <typename ...Options>
Pipe Pipe::grep(const std::string& pattern) {
   using AllowedOptions = opt::list<opt::i, opt::E>;
   static_assert(AllowedOptions::template contains_all<Options...>(), "Unknown option passed to Pipe.grep()");
   using GivenOptions = opt::list<Options...>;
   
   std::regex regex;
   
   if constexpr(GivenOptions::template contains<opt::i>()) {
      if constexpr(GivenOptions::template contains<opt::E>()) {
         regex = std::regex(pattern, std::regex::egrep | std::regex::icase);
      } else {
         regex = std::regex(pattern, std::regex::grep | std::regex::icase);
      }
   } else {
      if constexpr(GivenOptions::template contains<opt::E>()) {
         regex = std::regex(pattern, std::regex::egrep);
      } else {
         regex = std::regex(pattern, std::regex::grep);
      }
   }

   const auto is_not_match = [&regex](const std::string& str){ return not std::regex_search(str, regex); };
   
   const auto new_end = std::remove_if(lines.begin(), lines.end(), is_not_match);
   lines.erase(new_end, lines.end());
   
   return *this;
}

//
// Head
//

Pipe Pipe::head() {
   const uint8_t count = 10;
   if(lines.size() > count)
      lines.resize(count);

   return *this;
}

template <typename option>
Pipe Pipe::head(const size_t count) {
   static_assert(std::is_same_v<option, opt::n>, "Unknown option given to Pipe.head()");

   if(lines.size() > count)
      lines.resize(count);

   return *this;
}

template <>
Pipe Pipe::head<opt::c>(const size_t count) {
   if(count == 0) {
      lines.clear();
      return *this;
   }
   
   size_t line_count = 0;
   size_t char_count = 0;
   
   for(auto& line : lines) {
      ++line_count;
      char_count += line.size();
      if(char_count >= count) {
         auto extra_chars = char_count - count;
         
         if(extra_chars != 0)
            line.resize(line.size() - extra_chars);
         
         break;
      }
   }
   
   if(lines.size() > line_count)
      lines.resize(line_count);

   return *this;
}

//
// Sorhs
//

template <typename ...Options>
Pipe Pipe::sort() {
   using AllowedOptions = opt::list<opt::b, opt::d, opt::f, opt::r, opt::s, opt::u>;
   static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option passed to Pipe.sort()");
   using GivenOptions = opt::list<Options...>;
   
   if constexpr(GivenOptions::template contains<opt::s>()) {
      std::stable_sort(lines.begin(), lines.end(), sort_cmp<Options...>());
   } else {
      std::sort(lines.begin(), lines.end(), sort_cmp<Options...>());
   }
   
   if constexpr(GivenOptions::template contains<opt::u>()) {
      auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

template <typename ...Options>
Pipe Pipe::sort(const Pipe& other) {
   using AllowedOptions = opt::list<opt::b, opt::d, opt::f, opt::m, opt::r, opt::s, opt::u>;
   static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option passed to Pipe.sort()");
   using GivenOptions = opt::list<Options...>;
   static_assert(GivenOptions::template contains<opt::m>(), "-m option required for Pipe.sort() merge method");
   
   std::swap(lines, merge(this, other, sort_cmp<Options...>()));
   
   if constexpr(GivenOptions::template contains<opt::u>()) {
      auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

//
// Tail
//
Pipe Pipe::tail() {
   const uint8_t count = 10;
   if(lines.size() > count)
      lines = std::vector(lines.end() - count, lines.end());
   
   return *this;
}

template <typename option>
Pipe Pipe::tail(const size_t count) {
   static_assert(std::is_same_v<option, opt::n>, "Unknown option given to Pipe.tail()");

   if(lines.size() > count)
      lines = std::vector(lines.end() - count, lines.end());

   return *this;
}

template <>
Pipe Pipe::tail<opt::c>(const size_t count) {
   if(count == 0) {
      lines.clear();
      return *this;
   }
   
   size_t line_count = 0;
   size_t char_count = 0;
   
   for(auto rev_it = lines.rbegin(); rev_it != lines.rend(); ++rev_it) {
      ++line_count;
      char_count += rev_it->size();
      if(char_count >= count) {
         auto extra_chars = char_count - count;
         
         if(extra_chars != 0)
            *rev_it = std::string(rev_it->begin() + extra_chars, rev_it->end());
         
         break;
      }
   }
   
   if(lines.size() > line_count)
      lines = std::vector(lines.end() - line_count, lines.end());

   return *this;
}

//
// Tr
//

//TODO: Explore adding support for regex constants a la [:alpha:] and [a-z].

//TODO: Add support for -ds option which applies d and then s. Also, option c should be supported here as well.
Pipe Pipe::tr(const std::string& pattern1, const std::string& pattern2) {
   std::map<char, char> translation_map;
   
   auto it1 = pattern1.begin();
   auto it2 = pattern2.begin();
   for(; it1 != pattern1.end(); ++it1) {
      if(it2 == pattern2.end()) {
         translation_map[*it1] = pattern2.back();
      } else {
         translation_map[*it1] = *it2;
         ++it2;
      }
   }
   
   auto translate = [&translation_map](char ch){
      return translation_map.count(ch) ? translation_map[ch] : ch;
   };
   
   for(auto& line : lines) {
      std::transform(line.begin(), line.end(), line.begin(), translate);
   }

   return *this;
}

template <typename ...Options>
Pipe Pipe::tr(const std::string& pattern) {
   using RequiredOptions = opt::list<opt::d, opt::s>;
   static_assert(RequiredOptions::contains_any<Options...>(), "Pipe.tr() missing required option -d or -s");
   using AllowedOptions = opt::list<opt::d, opt::s, opt::c>;
   static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option given to Pipe.tr()");
   using GivenOptions = opt::list<Options...>;
   
   if constexpr(GivenOptions::template contains<opt::d>()) {
      std::function<bool(char)> match_pattern = [&pattern](char ch){
         return pattern.find(ch) != std::string::npos;
      };
      
      if constexpr(GivenOptions::template contains<opt::c>()) {
         match_pattern = std::not_fn(match_pattern);
      }
      
      for(auto& line : lines) {
         auto last = std::remove_if(line.begin(), line.end(), match_pattern);
         line.erase(last, line.end());
      }
   } else { // Option::s
      std::function<bool(char, char)> is_same_n_match = [&pattern](char first, char second){
         return first == second and pattern.find(first) != std::string::npos;
      };
      
      if constexpr(GivenOptions::template contains<opt::c>()) {
         is_same_n_match = std::not_fn(is_same_n_match);
      }
      
      for(auto& line : lines) {
         auto last = std::unique(line.begin(), line.end(), is_same_n_match);
         line.erase(last, line.end());
      }
   }

   return *this;
}

//
// Uniq
//

template <typename option = opt::none>
Pipe Pipe::uniq() {
   static_assert(std::is_same_v<option, opt::none>, "Unknown option given to Pipe.uniq()");
   
   if(not lines.empty()) {
      const auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

template <>
Pipe Pipe::uniq<opt::c>() {
   if(lines.empty()) {
      return *this;
   }
   
   auto last = lines.begin();
   auto curr = last;
   while(curr != lines.end()) {
      auto next = detail::find_next_diff(curr, lines.end());
      auto freq = std::distance(curr, next);
      
      curr->insert(0, detail::pad_left(freq, 4) + " ");
      
      if(curr != last)
         std::iter_swap(curr, last);
      ++last;
      
      curr = next;
   }
   
   lines.erase(last, lines.end());
   
   return *this;
}

template <>
Pipe Pipe::uniq<opt::d>() {
   if(lines.empty()) {
      return *this;
   }
   
   auto last = lines.begin();
   auto curr = last;
   while(curr != lines.end()) {
      auto next = detail::find_next_diff(curr, lines.end());
      auto freq = std::distance(curr, next);
      
      if(freq > 1) {
         if(curr != last)
            std::iter_swap(curr, last);
         
         ++last;
      }
      curr = next;
   }
   
   lines.erase(last, lines.end());
   
   return *this;
}

template <>
Pipe Pipe::uniq<opt::u>() {
   if(lines.empty()) {
      return *this;
   }
   
   auto last = lines.begin();
   auto curr = last;
   while(curr != lines.end()) {
      auto next = detail::find_next_diff(curr, lines.end());
      auto freq = std::distance(curr, next);
      
      if(freq == 1) {
         if(curr != last)
            std::iter_swap(curr, last);
         
         ++last;
      }
      curr = next;
   }
   
   lines.erase(last, lines.end());
   
   return *this;
}

//
// WC - Word Count
//

template <typename ...Options>
Pipe Pipe::wc() {
   using AllowedOptions = opt::list<opt::c, opt::l, opt::m, opt::w>;
   using GivenOptions = opt::list<Options...>;
   static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option given to Pipe.wc()");
   
   // Count chars, words and lines
   size_t num_chars{}, num_words{};
   const auto num_lines = lines.size();
   for(const auto& line : lines) {
      num_chars += line.size();
      
      // Count words in line
      auto it = line.begin();
      while(it != line.end()) {
         // Consume whitespace
         while(it != line.end() and std::isspace(*it))
            ++it;
         
         if(it != line.end())
            ++num_words;
         
         // Consume word
         while(it != line.end() and not std::isspace(*it))
            ++it;
      }
   }
   
   lines.clear();
   
   // Add results as string to output using the original
   // format of lines, words and chars.
   const int width = 8;
   std::string line;
   if constexpr(GivenOptions::empty() or GivenOptions::template contains<opt::l>()) {
      line += detail::pad_left(num_lines, width);
   }
   if constexpr(GivenOptions::empty() or GivenOptions::template contains<opt::w>()) {
      line += detail::pad_left(num_words, width);
   }
   if constexpr(GivenOptions::empty() or GivenOptions::template contains_any<opt::c, opt::m>()) {
      line += detail::pad_left(num_chars, width);
   }
   
   lines.push_back(line);
   
   return *this;
}

// PRIVATE

//
// Init
//
void Pipe::append(std::istream& in) {
   std::string buffer;
   while(std::getline(in, buffer)) {
      lines.push_back(std::move(buffer));
   }
}

//
// Helpers
//
void Pipe::number_lines(const bool skip_blank = false) {
   const short width = 4;
   size_t i = 1;
   
   if(skip_blank) {
      const auto blank_indent = std::string(width + 1, ' ');
      
      for(auto& line : lines) {
         if(not line.empty()) {
            line.insert(0, detail::pad_left(i, width) + " ");
         } else {
            line.insert(0, blank_indent);
         }
         ++i;
      }
   } else {
      for(auto& line : lines) {
         line.insert(0, detail::pad_left(i, width) + " ");
         ++i;
      }
   }
}

size_t Pipe::char_count() const {
   size_t count = 0;
   return std::accumulate(lines.begin(), lines.end(), count,
                          [](size_t count, const std::string& line){ return count + line.length(); });
}

std::vector<std::string> Pipe::merge(const Pipe& p1, const Pipe& p2, StringCmp cmp) {
   std::vector<std::string> dest;
   
   std::merge(p1.lines.begin(), p1.lines.end(),
              p2.lines.begin(), p2.lines.end(),
              std::back_inserter(dest), cmp);
   
   return dest;
}

template <typename ...Options>
Pipe::StringCmp Pipe::sort_cmp() {
   using GivenOptions = opt::list<Options...>;
   
   Pipe::StringCmp cmp;
   
   if constexpr(GivenOptions::template contains_all<opt::d, opt::f>()) {
      cmp = [](const std::string_view lhs, const std::string_view rhs) {
         auto lhs_it = lhs.begin();
         auto rhs_it = rhs.begin();
         
         for(;;) {
            while(lhs_it != lhs.end() and not std::isalnum(*lhs_it) and not std::isspace(*lhs_it))
               ++lhs_it;
            
            while(rhs_it != rhs.end() and not std::isalnum(*rhs_it) and not std::isspace(*rhs_it))
               ++rhs_it;
            
            if(lhs_it == lhs.end() or rhs_it == rhs.end()
               or std::toupper(*lhs_it) != std::toupper(*rhs_it)) {
               break;
            }
            
            ++lhs_it;
            ++rhs_it;
         }
         
         if(lhs_it == lhs.end()) {
            return true;
         } else if(rhs_it == rhs.end()) {
            return false;
         } else {
            return std::toupper(*lhs_it) < std::toupper(*rhs_it);
         }
      };
   } else if constexpr(GivenOptions::template contains<opt::d>()) {
      cmp = [](const std::string_view lhs, const std::string_view rhs) {
         auto lhs_it = lhs.begin();
         auto rhs_it = rhs.begin();
         
         for(;;) {
            while(lhs_it != lhs.end() and not std::isalnum(*lhs_it) and not std::isspace(*lhs_it))
               ++lhs_it;
            
            while(rhs_it != rhs.end() and not std::isalnum(*rhs_it) and not std::isspace(*rhs_it))
               ++rhs_it;
            
            if(lhs_it == lhs.end() or rhs_it == rhs.end() or *lhs_it != *rhs_it)
               break;
            
            ++lhs_it;
            ++rhs_it;
         }
         
         if(lhs_it == lhs.end()) {
            return true;
         } else if(rhs_it == rhs.end()) {
            return false;
         } else {
            return *lhs_it < *rhs_it;
         }
      };
   } else if constexpr(GivenOptions::template contains<opt::f>()) {
      cmp = [](const std::string_view lhs, const std::string_view rhs) {
         auto lhs_it = lhs.begin();
         auto rhs_it = rhs.begin();
         
         while(lhs_it != lhs.end() and rhs_it != rhs.end()
               and std::toupper(*lhs_it) == std::toupper(*rhs_it)) {
            ++lhs_it;
            ++rhs_it;
         }
         
         if(lhs_it == lhs.end()) {
            return true;
         } else if(rhs_it == rhs.end()) {
            return false;
         } else {
            return *lhs_it < *rhs_it;
         }
      };
   } else {
      cmp = std::less<const std::string_view>();
   }
   
   if constexpr(GivenOptions::template contains<opt::b>()) {
      cmp = [cmp](const std::string_view lhs, const std::string_view rhs) {
         return cmp(detail::skip_whitespace(lhs), detail::skip_whitespace(rhs));
      };
   }
   
   if constexpr(GivenOptions::template contains<opt::r>()) {
      cmp = std::not_fn(cmp);
   }
   
   return cmp;
}

//
// Output
//
void Pipe::pipe_to(std::ostream& os) {
   for(const auto& line : lines)
      os << line << '\n';
}

} // namespace pipe
