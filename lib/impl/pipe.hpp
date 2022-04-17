#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string_view>

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
   using StringCmp = std::function<bool(const std::string&, const std::string&)>;
   
   ~Pipe() = default;
   
   /*
    
    Output Operators
    
    */
   
   // Overwrites File with piped lines.
   void operator>(const File& file);
   // Appends piped lines to File.
   void operator>>(const File& file);
   // Append to ostream.
   void operator|(std::ostream& os);
   // Pipe to Tee (append or write options specified in Tee class).
   template<typename tee_option>
   void operator|(Tee<tee_option>& tee);
   
   /*
    
    Pipe Filters
    
    */
   
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
   
   Pipe tail(const int count);
   
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
   void append(std::istream& in);
   
   //
   // Helpers
   //
   void number_lines(const bool skip_blank);
   
   static std::vector<std::string> merge(const Pipe& p1, const Pipe& p2, StringCmp cmp);
   template <typename ...Options>
   StringCmp sort_cmp();

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
// Sort
//

template <typename ...Options>
Pipe Pipe::sort() {
   using AllowedOptions = opt::list<opt::b, opt::d, opt::f, opt::r, opt::u>;
   static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option passed to Pipe.sort()");
   using GivenOptions = opt::list<Options...>;
   
   std::sort(lines.begin(), lines.end(), sort_cmp<Options...>());
   
   if constexpr(GivenOptions::template contains<opt::u>()) {
      std::unique(lines.begin(), lines.end());
   }
   
   return *this;
}

template <typename ...Options>
Pipe Pipe::sort(const Pipe& other) {
   using AllowedOptions = opt::list<opt::b, opt::d, opt::f, opt::m, opt::r, opt::u>;
   static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option passed to Pipe.sort()");
   using GivenOptions = opt::list<Options...>;
   static_assert(GivenOptions::template contains<opt::m>(), "-m option NOT passed to Pipe.sort() merge method");
   
   std::swap(lines, merge(this, other, sort_cmp<Options...>()));
   
   if constexpr(GivenOptions::template contains<opt::u>()) {
      std::unique(lines.begin(), lines.end());
   }
   
   return *this;
}

//
// Tail
//

Pipe Pipe::tail(const int count = 10) {
   if(lines.size() > count)
      lines = std::vector(lines.end() - count, lines.end());
   
   return *this;
}

//
// Uniq
//

template <typename option = opt::none>
Pipe Pipe::uniq() {
   static_assert(std::is_same_v<option, opt::none>, "Unknown option given to Pipe.uniq()");
   
   if(!lines.empty()) {
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
         while(it != line.end() && std::isspace(*it))
            ++it;
         
         if(it != line.end())
            ++num_words;
         
         // Consume word
         while(it != line.end() && !std::isspace(*it))
            ++it;
      }
   }
   
   lines.clear();
   
   // Add results as string to output using the original
   // format of lines, words and chars.
   const int width = 8;
   std::string line;
   if constexpr(GivenOptions::empty() || GivenOptions::template contains<opt::l>()) {
      line += detail::pad_left(num_lines, width);
   }
   if constexpr(GivenOptions::empty() || GivenOptions::template contains<opt::w>()) {
      line += detail::pad_left(num_words, width);
   }
   if constexpr(GivenOptions::empty() || GivenOptions::template contains_any<opt::c, opt::m>()) {
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
   
   if constexpr(GivenOptions::template contains<opt::b>()) {
      cmp = [](const std::string& a, const std::string& b) {
         int a_idx = 0;
         while(a_idx < a.size() && std::isspace(a[a_idx]))
            ++a_idx;
         
         int b_idx = 0;
         while(b_idx < b.size() && std::isspace(b[b_idx]))
            ++b_idx;
         
         return a.compare(a_idx, a.size(), b, b_idx) < 0;
      };
   } else {
      cmp = std::less<const std::string&>();
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
