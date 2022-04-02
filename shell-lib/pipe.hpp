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

namespace sh {

class Pipe {
public:
   //
   // Comstructors
   //
   Pipe(std::istream& input) {
      init(input);
   }
   ~Pipe() = default;
   
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
   void cat(const std::string& filepath, const bool append = false) {
      auto outfile = append
         ? std::ofstream(filepath, std::ios::out | std::ios::app)
         : std::ofstream(filepath);
      
      pipe(outfile);
   }
   
   void pipe(std::ostream& os) {
      for(const auto& line : lines)
         os << line << '\n';
   }
   
   void tee(std::ostream& os1, std::ostream& os2) {
      for(const auto& line : lines) {
         os1 << line << '\n';
         os2 << line << '\n';
      }
   }
   
protected:
   Pipe() = default;
   
   void init(std::istream& in) {
      std::string buffer;
      while(std::getline(in, buffer)) {
         lines.push_back(std::move(buffer));
      }
   }

private:
   std::vector<std::string> lines;
};

class Cat : Pipe {
   explicit Cat(std::string filepath) {
      std::ifstream infile(filepath);
      
      if(infile.is_open()) {
         init(infile);
      }
   }
};

class Echo : Pipe {
   explicit Echo(std::string data) {
      std::istringstream instring(data);
      init(instring);
   }
};

} //namespace sh
