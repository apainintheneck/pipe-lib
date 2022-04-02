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

namespace opt {

class _default {};

class a {};
class n {};

}

namespace sh {

class File {
public:
   File(const std::string& filename) : _filename(filename) {}
   File(std::string&& filename) : _filename(std::move(filename)) {}
   ~File() = default;
   
   friend class Pipe;
   
private:
   const std::string _filename;
};

template <typename option = opt::_default>
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
      if constexpr(std::is_same_v<option, opt::_default>) {
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
      pipe(outfile);
   }
   
   void operator>>(const File& file) {
      std::ofstream outfile(file._filename, std::ios::out | std::ios::app);
      pipe(outfile);
   }
   
   void operator|(std::ostream& os) {
      pipe(os);
   }
   
   template<typename tee_option>
   void operator|(Tee<tee_option>& tee) {
      for(const auto& line : lines) {
         tee << line;
         tee << '\n';
      }
   }
   
   //
   // Manipulation
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
   
   Pipe uniq() {
      if(lines.size() > 1) {
         auto prev = lines.begin();
         auto curr = prev + 1;
         
         while(curr != lines.end()) {
            if(*prev == *curr) {
               curr = lines.erase(curr);
               prev = curr - 1;
            } else {
               ++prev;
               ++curr;
            }
         }
      }
      
      return *this;
   }
   
   //
   // Output
   //
   void pipe(std::ostream& os) {
      for(const auto& line : lines)
         os << line << '\n';
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
   // Variables
   //
   std::vector<std::string> lines;
};

template <typename option = opt::_default>
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

template <typename option = opt::_default>
class Echo : public Pipe {
public:
   template <typename... Strings>
   Echo(Strings... strings) {
      std::string joined_strings;
      if constexpr(std::is_same_v<option, opt::_default>) {
         joined_strings = (add_newline(strings) + ...);
      } if constexpr(std::is_same_v<option, opt::n>) {
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
