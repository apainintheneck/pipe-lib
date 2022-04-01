//
//  streams.hpp
//  shell-lib
//
//  Created by Kevin on 3/31/22.
//

#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace sh {

void cat(std::istream& is, std::ostream& os = std::cout) {
   os << is.rdbuf();
}

bool cat(const std::string& filename, std::ostream& os = std::cout) {
   std::ifstream file(filename);
   
   if(file.is_open()) {
      cat(file, os);
      return true;
   }
   
   return false;
}

void echo(const char* input, std::ostream& os = std::cout) noexcept {
   os << input;
}

bool head(std::istream& is, size_t count = 10, std::ostream& os = std::cout) {
   std::string buffer;
   while(std::getline(is, buffer) && count) {
      os << buffer << '\n';
      --count;
   }
   
   return true;
}

bool head(const std::string& filepath, size_t count = 10, std::ostream& os = std::cout) {
   std::ifstream file(filepath);
   if(!file.is_open())
      return false;

   head(file, count, os);
   return true;
}

//Also, it might be worth exploring heap sort since you're getting one line at a time.
void sort(std::istream& is, std::ostream& os = std::cout) {
   std::string buffer;
   std::vector<std::string> lines;
   while(std::getline(is, buffer)) {
      lines.push_back(buffer);
   }
   
   std::sort(lines.begin(), lines.end());
   
   for(const auto& line: lines)
      os << line;
}

bool sort(const std::string& filepath, std::ostream& os = std::cout) {
   std::ifstream file(filepath);
   if(!file.is_open())
      return false;

   sort(file, os);
   return true;
}

void tail(std::istream& is, const size_t count = 10, std::ostream& os = std::cout) {
   std::vector<std::string> lines;
   lines.reserve(count);
   
   std::string buffer;
   for(int i = 0; i < count && std::getline(is, buffer); ++i)
      lines.push_back(buffer);
   
   size_t idx = 0;
   for(; std::getline(is, buffer); ++idx)
      lines[idx % count] = std::move(buffer);
   
   if(lines.size() < count){
      for(const auto& line: lines)
         os << line << '\n';
   } else {
      idx %= count;
      for(size_t offset = 0; offset < count; ++offset)
         os << lines[(idx + offset) % count] << '\n';
   }
}

bool tail(const std::string& filepath, const size_t count = 10, std::ostream& os = std::cout) {
   std::ifstream file(filepath);
   if(!file.is_open())
      return false;

   tail(file, count, os);
   return true;
}

class tee {
public:
   tee(std::ostream& os1, std::ostream& os2) : os1_(os1), os2_(os2) {}
   
   template <typename T>
   tee& operator<<(const T& value) {
      os1_ << value;
      os2_ << value;
      
      return *this;
   }
   
private:
   std::ostream& os1_;
   std::ostream& os2_;
};

std::string env(const char* input) noexcept {
   return std::getenv(input);
}

void uniq(std::istream& is, std::ostream& os = std::cout) {
   std::string prev;
   if(std::getline(is, prev))
      os << prev << '\n';
   
   std::string buffer;
   while(std::getline(is, buffer)) {
      if(buffer == prev) continue;
      
      os << buffer << '\n';
      prev = std::move(buffer);
   }
}

bool uniq(const std::string& filepath, std::ostream& os = std::cout) {
   std::ifstream file(filepath);
   if(!file.is_open())
      return false;

   uniq(file, os);
   return true;
}

} //namespace sh
