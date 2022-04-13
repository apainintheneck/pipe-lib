#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "option.hpp"
#include "util.hpp"
#include "output.hpp"

//
// The base class for all the following pipe classes.
//
namespace pipe {

class Pipe {
   friend class Builder;
public:
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
   // Used for error checking options.
   template <typename option>
   Pipe head(const size_t count);
   // Option::n - Retains the top x lines.
   template <>
   Pipe head<opt::n>(const size_t count);
   // Option::c - Retains the top x bytes.
   template <>
   Pipe head<opt::c>(const size_t count);
   
   //
   // Sort
   //
   
   Pipe sort() {
      std::sort(lines.begin(), lines.end());
      
      return *this;
   }
   
   //
   // Tail
   //
   
   Pipe tail(const int count = 10) {
      if(lines.size() > count)
         lines = std::vector(lines.end() - count, lines.end());
      
      return *this;
   }
   
   //
   // Uniq
   //
   
   // Defaults to removing adjacent duplicate lines.
   Pipe uniq();
   // Used for error checking options.
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
   
protected:
   Pipe() = default;
   
   //
   // Init
   //
   void append(std::istream& in) {
      std::string buffer;
      while(std::getline(in, buffer)) {
         lines.push_back(std::move(buffer));
      }
   }

private:
   //
   // Output
   //
   void pipe_to(std::ostream& os) {
      for(const auto& line : lines)
         os << line << '\n';
   }
   
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
   using AllowedOptions = opt::list<opt::n, opt::c>;
   static_assert(AllowedOptions::contains<option>(), "Unknown option given to Pipe.head()");
}

template <>
Pipe Pipe::head<opt::n>(const size_t count) {
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
// Uniq
//

Pipe Pipe::uniq() {
   if(!lines.empty()) {
      const auto last = std::unique(lines.begin(), lines.end());
      lines.erase(last, lines.end());
   }
   
   return *this;
}

template <typename option>
Pipe Pipe::uniq() {
   using AllowedOptions = opt::list<opt::c, opt::d, opt::u>;
   static_assert(AllowedOptions::contains<option>(), "Unknown option given to Pipe.uniq()");
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

} // namespace pipe
