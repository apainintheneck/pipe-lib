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
// The basic class Pipe class.
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
   void operator|(Tee& tee);
   
   /*
    
    Utils
    
    */
   
   // Number of lines in pipe.
   size_t size() const { return lines.size(); }
   size_t length() const { return lines.size(); }
   
   /*
    
    Pipe Filters
    
    */
   
   //
   // Fold
   //
   
   template <typename option>
   Pipe& fold();
   template <typename ...Options>
   Pipe& fold(const size_t len);
   
   //
   // Grep
   //
   
   // Defaults to basic grep.
   // Option::i - Ignore case
   // Option::E - egrep (extended grep)
   template <typename ...Options>
   Pipe& grep(const std::string& pattern);
   
   //
   // Head
   //
   
   // Defaults to retaining the top 10 lines.
   Pipe& head();
   // Option::n - Retains the top x lines. [Base case]
   template <typename option>
   Pipe& head(const size_t count);
   // Option::c - Retains the top x bytes.
   template <>
   Pipe& head<opt::c>(const size_t count);
   
   //
   // Paste
   //
   
   // TODO: Add -s option.
   
   // Default combine two pipes line-by-line with the tab character
   Pipe& paste(const Pipe& pipe);
   // Option::d takes a list of separators which are used instead of tab
   template <typename option>
   Pipe& paste(const std::string& separators, const Pipe& pipe);
   
   //
   // Sort
   //
   
   // TODO: Add more sort options.
   
   // Default lexical sort. [Base case]
   template <typename ...Options>
   Pipe& sort();
   // Option::m - Merge two selected ranges by comparator.
   template <typename ...Options>
   Pipe& sort(const Pipe& other);
   
   //
   // Tail
   //
   
   // Defaults to retaining the bottom 10 lines.
   Pipe& tail();
   // Option::n - Retains the bottom x lines. [Base case]
   template <typename option>
   Pipe& tail(const size_t count);
   // Option::c - Retains the bottom x bytes.
   template <>
   Pipe& tail<opt::c>(const size_t count);
   
   //
   // Tr
   //
   
   // Defaults to translating characters from pattern1 to pattern2
   // based upon their position.
   Pipe& tr(const std::string& pattern1, const std::string& pattern2);
   // Covers the one argument uses of the tr cmd.
   // Requires Option::d or Option::s with optional Option::c
   // Option::d - Deletes matching characters
   // Option::s - Squeezes adjacent matching characters (like unique but only for matching chars).
   // Option::c - Use the compliment of the given pattern.
   template <typename ...Options>
   Pipe& tr(const std::string& pattern);
   
   //
   // Uniq
   //
   
   // TODO: Add -i, -s and - f options as well.
   
   // Defaults to removing adjacent duplicate lines. [Base case]
   template <typename option>
   Pipe& uniq();
   // Option::c - Prefix a count of the frequency of each line.
   template <>
   Pipe& uniq<opt::c>();
   // Option::d - Only retain duplicate lines.
   template <>
   Pipe& uniq<opt::d>();
   // Option::d - Only retain unique lines.
   template <>
   Pipe& uniq<opt::u>();
   
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
   void number_lines();
   // Number all non blank lines in lines vector.
   void number_non_blank_lines();
   // Squeeze consecutive blank lines down to one.
   void squeeze_blank_lines();
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

void Pipe::operator|(Tee& tee) {
   for(const auto& line : lines) {
      tee << line << '\n';
   }
}

/*
 
 Pipe Filters
 
 */

//
// Fold
//

template <typename ...Options>
Pipe& Pipe::fold(const size_t len) {
   using OptionList = opt::list<Options...>;
   static_assert(OptionList::template allows<opt::w, opt::s, opt::none>(),
                 "Unknown option passed to Pipe.fold()");
   static_assert(OptionList::template requires<opt::w>(),
                 "Missing required option -w with call to Pipe.fold() with custom length");
   
   size_t end;
   std::vector<std::string> new_lines;
   new_lines.reserve(lines.size() * 2); // just a random number here
   
   for(auto& line : lines) {
      if(line.size() <= len) {
         new_lines.push_back(std::move(line));
      } else {
         size_t start = 0;
         
         while(start < line.size()) {
            if constexpr(OptionList::template contains<opt::s>()) {
               end = detail::line_len_with_end_blank(line.cbegin(), line.cend(), len);
            } else {
               end = detail::line_len(line.cbegin(), line.cend(), len);
            }
            
            new_lines.push_back(line.substr(start, end));
            start += len;
         }
      }
   }
   
   std::swap(lines, new_lines);
   
   return *this;
}

template <typename option = opt::none>
Pipe& Pipe::fold() {
   using AllowedOptions = opt::list<opt::none, opt::s>;
   static_assert(AllowedOptions::contains<option>(),
                 "Unknown option passed to Pipe.fold()");
   
   return this->fold<opt::w, option>(80);
}

//
// Grep
//

template <typename ...Options>
Pipe& Pipe::grep(const std::string& pattern) {
   using OptionList = opt::list<Options...>;
   static_assert(OptionList::template allows<opt::i, opt::E>(),
                 "Unknown option passed to Pipe.grep()");
   
   std::regex regex;
   
   if constexpr(OptionList::template contains<opt::i>()) {
      if constexpr(OptionList::template contains<opt::E>()) {
         regex = std::regex(pattern, std::regex::egrep | std::regex::icase);
      } else {
         regex = std::regex(pattern, std::regex::grep | std::regex::icase);
      }
   } else {
      if constexpr(OptionList::template contains<opt::E>()) {
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

Pipe& Pipe::head() {
   return this->head<opt::n>(10);
}

template <typename option>
Pipe& Pipe::head(const size_t count) {
   static_assert(std::is_same_v<option, opt::n>, "Unknown option given to Pipe.head()");

   if(lines.size() > count)
      lines.resize(count);

   return *this;
}

template <>
Pipe& Pipe::head<opt::c>(const size_t count) {
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
// Paste
//

// TODO: Add the ability to paste multiple files at the same time.

Pipe& Pipe::paste(const Pipe& pipe) {
   return this->paste<opt::d>("\t", pipe);
}

template <typename option>
Pipe& Pipe::paste(const std::string& separators, const Pipe& other) {
   static_assert(std::is_same_v<option, opt::d>,
                 "Unknown option passed to Pipe.paste()");
   
   size_t line_num = 0;
   for(const auto& new_line : other.lines) {
      if(line_num < lines.size()) {
         lines[line_num].append(separators[line_num % separators.size()] + new_line);
      } else {
         lines.push_back('\t' + new_line);
      }
      
      ++line_num;
   }
   
   for(; line_num < lines.size(); ++line_num)
      lines[line_num].push_back('\t');
   
   return *this;
}

//
// Sort
//

template <typename ...Options>
Pipe& Pipe::sort() {
   using OptionList = opt::list<Options...>;
   static_assert(OptionList::template allows<opt::b, opt::d, opt::f, opt::r, opt::s, opt::u>(),
                 "Unknown option passed to Pipe.sort()");
   
   if constexpr(OptionList::template contains<opt::s>()) {
      std::stable_sort(lines.begin(), lines.end(), sort_cmp<Options...>());
   } else {
      std::sort(lines.begin(), lines.end(), sort_cmp<Options...>());
   }
   
   if constexpr(OptionList::template contains<opt::u>()) {
      auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

// Implementation of the merge option (-m)
template <typename ...Options>
Pipe& Pipe::sort(const Pipe& other) {
   using OptionList = opt::list<Options...>;
   static_assert(OptionList::template requires<opt::m>(),
                 "Missing option -m with call to Pipe.sort() merge method");
   static_assert(OptionList::template allows<opt::b, opt::d, opt::f, opt::m, opt::r, opt::s, opt::u>(),
                 "Unknown option passed to Pipe.sort()");
   
   std::swap(lines, merge(this, other, sort_cmp<Options...>()));
   
   if constexpr(OptionList::template contains<opt::u>()) {
      auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

//
// Tail
//

Pipe& Pipe::tail() {
   return this->tail<opt::n>(10);
}

template <typename option>
Pipe& Pipe::tail(const size_t count) {
   static_assert(std::is_same_v<option, opt::n>, "Unknown option given to Pipe.tail()");

   if(lines.size() > count)
      lines = std::vector(lines.end() - count, lines.end());

   return *this;
}

template <>
Pipe& Pipe::tail<opt::c>(const size_t count) {
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

//TODO: Explore adding support for [abc].

//TODO: Add support for -ds option which applies d and then s. Also, option c should be supported here as well.
Pipe& Pipe::tr(const std::string& pattern1, const std::string& pattern2) {
   std::map<char, char> translation_map;
   
   auto expanded_pattern1 = detail::expand_tr_pattern(pattern1);
   auto expanded_pattern2 = detail::expand_tr_pattern(pattern2);
   
   auto it1 = expanded_pattern1.begin();
   auto it2 = expanded_pattern2.begin();
   for(; it1 != expanded_pattern1.end(); ++it1) {
      if(it2 == expanded_pattern2.end()) {
         translation_map[*it1] = expanded_pattern2.back();
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
Pipe& Pipe::tr(const std::string& pattern) {
   using OptionList = opt::list<Options...>;
   static_assert(OptionList::template allows<opt::d, opt::s, opt::c>(),
                 "Unknown option given to Pipe.tr()");
   static_assert(OptionList::template requires<opt::d, opt::s>(1),
                 "Pipe.tr() missing required option -d or -s");
   
   auto expanded_pattern = detail::expand_tr_pattern(pattern);
   
   if constexpr(OptionList::template contains<opt::d>()) {
      std::function<bool(char)> match_pattern = [&expanded_pattern](char ch){
         return expanded_pattern.find(ch) != std::string::npos;
      };
      
      if constexpr(OptionList::template contains<opt::c>()) {
         match_pattern = std::not_fn(match_pattern);
      }
      
      for(auto& line : lines) {
         auto last = std::remove_if(line.begin(), line.end(), match_pattern);
         line.erase(last, line.end());
      }
   } else { // Option::s
      std::function<bool(char, char)> is_same_and_match = [&expanded_pattern](char first, char second){
         return first == second and expanded_pattern.find(first) != std::string::npos;
      };
      
      if constexpr(OptionList::template contains<opt::c>()) {
         is_same_and_match = std::not_fn(is_same_and_match);
      }
      
      for(auto& line : lines) {
         auto last = std::unique(line.begin(), line.end(), is_same_and_match);
         line.erase(last, line.end());
      }
   }

   return *this;
}

//
// Uniq
//

template <typename option = opt::none>
Pipe& Pipe::uniq() {
   static_assert(std::is_same_v<option, opt::none>, "Unknown option given to Pipe.uniq()");
   
   if(not lines.empty()) {
      const auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

template <>
Pipe& Pipe::uniq<opt::c>() {
   if(lines.empty()) {
      return *this;
   }
   std::vector<unsigned short> freq_list;
   
   auto last = lines.begin();
   auto curr = last;
   while(curr != lines.end()) {
      auto next = detail::find_next_diff(curr, lines.end());
      freq_list.push_back(std::distance(curr, next));
      
      if(curr != last)
         std::iter_swap(curr, last);
      ++last;
      
      curr = next;
   }
   
   lines.erase(last, lines.end());
   
   auto width = detail::count_digits(*std::max_element(freq_list.begin(), freq_list.end()));
   
   auto freq_it = freq_list.begin();
   for(auto& line : lines) {
      line.insert(0, detail::pad_left(*freq_it, width) + " ");
      ++freq_it;
   }
   
   return *this;
}

template <>
Pipe& Pipe::uniq<opt::d>() {
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
Pipe& Pipe::uniq<opt::u>() {
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

// PRIVATE

//
// Init
//

void Pipe::append(std::istream& in) {
   std::string buffer;
   
   while(std::getline(in, buffer)) {
      lines.push_back(std::move(buffer));
      // Ignore trailing carriage return after a newline.
      //    Ex. "\n\r"
      if(in.peek() == '\r') in.ignore();
   }
}

//
// Helpers
//

void Pipe::number_lines() {
   const short width = detail::count_digits(lines.size());
   size_t i = 1;
   
   for(auto& line : lines) {
      line.insert(0, detail::pad_left(i, width) + " ");
      ++i;
   }
}

void Pipe::number_non_blank_lines() {
   const short width = detail::count_digits(lines.size());
   size_t i = 1;
   
   for(auto& line : lines) {
      if(not line.empty()) {
         line.insert(0, detail::pad_left(i, width) + " ");
         ++i;
      }
   }
}

void Pipe::squeeze_blank_lines() {
   const auto consecutive_blank_lines = [](const auto& a, const auto& b){ return a.empty() and b.empty(); };
   
   const auto last = std::unique(lines.begin(), lines.end(), consecutive_blank_lines);
   lines.erase(last, lines.end());
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
   using OptionList = opt::list<Options...>;
   
   Pipe::StringCmp cmp;
   
   if constexpr(OptionList::template contains<opt::d, opt::f>()) {
      // Case insensitive dictionary compare (only compare uppercase alnum characters or blank spaces)
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
   } else if constexpr(OptionList::template contains<opt::d>()) {
      // Dictionary order compare (only compare blank spaces and alnum characters)
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
   } else if constexpr(OptionList::template contains<opt::f>()) {
      // Case insensitive compare (everything converted to uppercase)
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
   
   if constexpr(OptionList::template contains<opt::b>()) {
      // Skip leading whitespace before comparing
      cmp = [cmp](const std::string_view lhs, const std::string_view rhs) {
         return cmp(detail::skip_whitespace(lhs), detail::skip_whitespace(rhs));
      };
   }
   
   if constexpr(OptionList::template contains<opt::r>()) {
      // Sort in reverse order (negate the compare function)
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
