//
//  streams.hpp
//  shell-lib
//
//  Created by Kevin on 3/31/22.
//

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

//
// Metaprogramming Helpers
//
namespace opt {
// Command line options are passed as undefined class types
// that get evaluated at compile time.
class opt {};
class a {};
class c {};
class d {};
class l {};
class m {};
class n {};
class w {};
}

// A helper that takes a list of options types and validates them.
template <typename ...Options>
struct opt_list {
   template <typename option>
   static constexpr bool contains() {
      return (std::is_same_v<option, Options> || ...);
   }
   
   template <typename ...OptionsToTest>
   static constexpr bool contains_all() {
      return (contains<OptionsToTest>() && ...);
   }
   
   template <typename ...OptionsToTest>
   static constexpr bool contains_any() {
      return (contains<OptionsToTest>() || ...);
   }
   
   static constexpr bool empty() {
      return sizeof... (Options) == 0;
   }
};

namespace sh {

// A simple file class that is used to output the value of a pipe
// by mimicking shell syntax with > and >>.
//
// It can be used to output to a new file or overwrite an existing one.
// Ex. sh::Pipe(is) > sh::File("example.txt");
//
// Or just append to the end of an existing file.
// Ex. sh::Pipe(is) >> sh::File("example.txt");
class File {
public:
   File(const std::string& filename) : _filename(filename) {}
   File(std::string&& filename) : _filename(std::move(filename)) {}
   ~File() = default;
   
   friend class Pipe;
   
private:
   const std::string _filename;
};

// A replacement for the tee command by writing to an output stream,
// writing or appending to files and mimicking shell syntax with |.
//
// It can be used with one file and an outuput stream (the default is std::cout).
// Ex. sh::Pipe(is) | sh::Tee("example.txt"); // Overwrite example
// Ex. sh::Pipe(is) | sh::Tee(os, "example.txt"); // Explicit ostream example
// Ex. sh::Pipe(is) | sh::Tee<opt::a>("example.txt"); // Append example
//
// Or with multiple files at once.
// Ex. sh::Pipe(is) | sh::Tee("example.txt", "another.txt", "one_more.txt");
template <typename option = opt::opt>
class Tee {
public:
   template <typename... Filenames>
   Tee(Filenames... files) : _out(std::cout) {
      (add_ofstream(files), ...);
   }
   template <typename... Filenames>
   Tee(std::ostream& os, Filenames... files) : _out(os) {
      (add_ofstream(files), ...);
   }
   ~Tee() = default;
   
   friend class Pipe;
   
private:
   //
   // Operators
   //
   void operator<<(const std::string& str) {
      _out << str;
      
      for(auto& outfile : _outfiles)
         outfile << str;
   }
   
   void operator<<(const char ch) {
      _out << ch;
      
      for(auto& outfile : _outfiles)
         outfile << ch;
   }

   //
   // Init
   //
   void add_ofstream(const std::string& file) {
      if constexpr(std::is_same_v<option, opt::opt>) {
         std::ofstream outfile(file);
         if(outfile.is_open())
            _outfiles.push_back(std::move(outfile));
      } else if constexpr(std::is_same_v<option, opt::a>) {
         std::ofstream outfile(file, std::ios::out | std::ios::app);
         if(outfile.is_open())
            _outfiles.push_back(std::move(outfile));
      }
   }
   
   //
   // Variables
   //
   std::ostream& _out;
   std::vector<std::ofstream> _outfiles;
};

class Pipe {
public:
   Pipe(std::istream& input) {
      init(input);
   }
   ~Pipe() = default;
   
   //
   // Operators
   //
   void operator>(const File& file) {
      std::ofstream outfile(file._filename);
      pipe_to(outfile);
   }
   
   void operator>>(const File& file) {
      std::ofstream outfile(file._filename, std::ios::out | std::ios::app);
      pipe_to(outfile);
   }
   
   void operator|(std::ostream& os) {
      pipe_to(os);
   }
   
   template<typename tee_option>
   void operator|(Tee<tee_option>& tee) {
      for(const auto& line : lines) {
         tee << line;
         tee << '\n';
      }
   }
   
   //
   // Filters
   //
   Pipe head(const int count = 10) {
      if(lines.size() > count)
         lines.resize(count);

      return *this;
   }
   
   Pipe sort() {
      std::sort(lines.begin(), lines.end());
      
      return *this;
   }
   
   Pipe tail(const int count = 10) {
      if(lines.size() > count)
         lines = std::vector(lines.end() - count, lines.end());
      
      return *this;
   }
   
   template <typename option = opt::opt>
   Pipe uniq() {
      static_assert(std::is_same_v<option, opt::opt>, "Unknown option given to Pipe.uniq()");
      
      if(!lines.empty()) {
         const auto last = std::unique(lines.begin(), lines.end());
         lines.erase(last, lines.end());
      }
      
      return *this;
   }
   
   template <>
   Pipe uniq<opt::c>() {
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
   Pipe uniq<opt::d>() {
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
   Pipe wc() {
      using AllowedOptions = opt_list<opt::c, opt::l, opt::m, opt::w>;
      using GivenOptions = opt_list<Options...>;
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
   Pipe() = default;
   
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
   // Helpers
   //
   std::string pad_left(std::string&& str, const size_t width) const {
      return str.size() < width
         ? std::string(width - str.size(), ' ') + str
         : str;
   }
   
   template <typename T>
   std::string pad_left(const T value, const size_t width) const {
      return pad_left(std::to_string(value), width);
   }
   
   bool case_insensitive_cmp(const std::string_view a, const std::string_view b) {
      auto cmp_upper_char = [](const char a, const char b) {
         return std::toupper(a) == std::toupper(b);
      };
      
      return std::equal(a.begin(), a.end(),
                        b.begin(), b.end(),
                        cmp_upper_char);
   }
   
   template <typename iter>
   iter consume_whitespace(iter begin, iter end) const {
      while(begin != end && std::isspace(*begin))
         ++begin;
      
      return begin;
   }
   
   template <typename iter>
   iter consume_field(iter begin, iter end) const {
      while(begin != end && !std::isspace(*begin))
         ++begin;
      
      return begin;
   }
   
   template <typename iter>
   std::string_view get_field(iter begin, iter end) const {
      return std::string_view(begin, consume_field(begin, end));
   }
   
   std::string_view skip_n_fields(const std::string_view str, size_t n) const {
      auto iter = str.begin();
      while(iter != str.end() && n > 0) {
         iter = consume_whitespace(iter, str.end());
         iter = consume_field(iter, str.end());
         --n;
      }
      
      return str.substr(std::distance(str.begin(), iter));
   }
   
   std::string_view skip_n_chars(const std::string_view str, const size_t n) const {
      return str.substr(n);
   }
   
   //
   // Variables
   //
   std::vector<std::string> lines;
};

template <typename option = opt::opt>
class Cat : public Pipe {
public:
   template <typename... Filenames>
   Cat(Filenames... files) {
      (append_file_contents(files), ...);
   }
   ~Cat() = default;

private:
   //
   // Init
   //
   void append_file_contents(std::string filepath) {
      std::ifstream infile(filepath);
      
      if(infile.is_open()) {
         init(infile);
      }
   }
};

template <typename option = opt::opt>
class Echo : public Pipe {
public:
   template <typename... Strings>
   Echo(Strings... strings) {
      std::string joined_strings;
      if constexpr(std::is_same_v<option, opt::opt>) {
         joined_strings = (add_newline(strings) + ...);
      } else if constexpr(std::is_same_v<option, opt::n>) {
         joined_strings = (std::string(strings) + ...);
      }
      
      std::istringstream instring(joined_strings);
      init(instring);
   }
   ~Echo() = default;
   
private:
   //
   // Init
   //
   std::string add_newline(const std::string& str) {
      return str + "\n";
   }
};

} //namespace sh
