#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "option.hpp"
#include "util.hpp"
#include "output_pipe.hpp"

//
// The base class for all the following pipe classes.
//
namespace sh::detail {

class BasicPipe {
public:
   ~BasicPipe() = default;
   
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
   BasicPipe head();
   // Used for error checking options.
   template <typename option>
   BasicPipe head(const size_t count);
   // Option::n - Retains the top x lines.
   template <>
   BasicPipe head<opt::n>(const size_t count);
   // Option::c - Retains the top x bytes.
   template <>
   BasicPipe head<opt::c>(const size_t count);
   
   BasicPipe sort() {
      std::sort(lines.begin(), lines.end());
      
      return *this;
   }
   
   BasicPipe tail(const int count = 10) {
      if(lines.size() > count)
         lines = std::vector(lines.end() - count, lines.end());
      
      return *this;
   }
   
   template <typename option = opt::opt>
   BasicPipe uniq() {
      static_assert(std::is_same_v<option, opt::opt>, "Unknown option given to BasicPipe.uniq()");
      
      if(!lines.empty()) {
         const auto last = std::unique(lines.begin(), lines.end());
         lines.erase(last, lines.end());
      }
      
      return *this;
   }
   
   template <>
   BasicPipe uniq<opt::c>() {
      if(lines.empty())
         return *this;
      
      int freq = 1;
      auto prev = lines.begin();
      auto curr = prev + 1;
      
      while(curr != lines.end()) {
         if(*prev == *curr) {
            curr = lines.erase(curr);
            prev = curr - 1;
         } else {
            prev->insert(0, pad_left(freq, 4));
            ++prev;
            ++curr;
            freq = 1;
         }
      }
      prev->insert(0, pad_left(freq, 4));
      
      return *this;
   }
   
   template <>
   BasicPipe uniq<opt::d>() {
      if(lines.empty()) {
         return *this;
      } else if(lines.size() == 1) {
         lines.clear();
         return *this;
      }
      
      auto last = lines.begin();
      auto prev = lines.begin();
      auto curr = prev + 1;
      while(curr != lines.end()) {
         int freq = 1;
         while(curr != lines.end() && *prev == *curr) {
            ++prev;
            ++curr;
            ++freq;
         }
         
         if(freq > 1) {
            std::iter_swap(last, prev);
            ++last;
            
            if(curr == lines.end())
               break;
            
            ++prev;
            ++curr;
         }
      }
      
      if(last == lines.begin()) {
         lines.clear();
      } else {
         lines.erase(last, lines.end());
      }
      
      return *this;
   }
   
   template <typename ...Options>
   BasicPipe wc() {
      using AllowedOptions = opt::list<opt::c, opt::l, opt::m, opt::w>;
      using GivenOptions = opt::list<Options...>;
      static_assert(AllowedOptions::contains_all<Options...>(), "Unknown option given to BasicPipe.wc()");
      
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
         line += pad_left(num_lines, width);
      }
      if constexpr(GivenOptions::empty() || GivenOptions::template contains<opt::w>()) {
         line += pad_left(num_words, width);
      }
      if constexpr(GivenOptions::empty() || GivenOptions::template contains_any<opt::c, opt::m>()) {
         line += pad_left(num_chars, width);
      }
      
      lines.push_back(line);
      
      return *this;
   }
   
protected:
   BasicPipe() = default;
   
   //
   // Init
   //
   void init(std::istream& in) {
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

void BasicPipe::operator>(const File& file) {
   std::ofstream outfile(file._filename);
   pipe_to(outfile);
}

void BasicPipe::operator>>(const File& file) {
   std::ofstream outfile(file._filename, std::ios::out | std::ios::app);
   pipe_to(outfile);
}

void BasicPipe::operator|(std::ostream& os) {
   pipe_to(os);
}

template<typename tee_option>
void BasicPipe::operator|(Tee<tee_option>& tee) {
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
BasicPipe BasicPipe::head() {
   const uint8_t count = 10;
   if(lines.size() > count)
      lines.resize(count);

   return *this;
}

template <typename option>
BasicPipe BasicPipe::head(const size_t count) {
   using AllowedOptions = opt::list<opt::n, opt::c>;
   static_assert(AllowedOptions::contains<option>(), "Unknown option given to BasicPipe.head()");
}

template <>
BasicPipe BasicPipe::head<opt::n>(const size_t count) {
   if(lines.size() > count)
      lines.resize(count);

   return *this;
}

template <>
BasicPipe BasicPipe::head<opt::c>(const size_t count) {
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

} // namespace sh::detail
