#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>

#include "option.hpp"
#include "pipe.hpp"
#include "builder.hpp"

namespace pipe {

//
// Cat command
//
template <typename option = opt::none>
Pipe cat(const std::string& filename) {
   return cat<option>({filename});
}

template <typename option = opt::none>
Pipe cat(std::initializer_list<std::string> filenames) {
   static_assert(std::is_same_v<opt::none, option>, "Unknown option passed to cat()");
   
   auto builder = Builder();
   
   for(const auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   return builder.build();
}

template <>
Pipe cat<opt::n>(std::initializer_list<std::string> filenames) {
   auto builder = Builder();
   
   for(const auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   builder.number_lines();
   
   return builder.build();
}

template <>
Pipe cat<opt::b>(std::initializer_list<std::string> filenames) {
   auto builder = Builder();
   
   for(const auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   builder.number_lines(true);
   
   return builder.build();
}

//
// Echo command
//
template <typename option = opt::none>
Pipe echo(const std::string& str) {
   return echo<option>({str});
}

template <typename option = opt::none>
Pipe echo(std::initializer_list<std::string> strs) {
   static_assert(std::is_same_v<opt::none, option>, "Unknown option passed to echo()");
   
   std::istringstream input(std::accumulate(strs.begin(), strs.end(), std::string()));
   
   auto builder = Builder();
   builder.append(input);
   return builder.build();
}

template <>
Pipe echo<opt::n>(std::initializer_list<std::string> strs) {
   if(strs.size() > 0) {
      const auto concat_with_newline = [](std::string a, std::string b) { return a + "\n" + b; };
      std::istringstream input(std::accumulate(strs.begin() + 1, strs.end(), *strs.begin(), concat_with_newline));
      
      auto builder = Builder();
      builder.append(input);
      return builder.build();
   } else {
      return Builder().build();
   }
}

//
// Stream command
//
template <typename ...IStream>
Pipe stream(IStream& ...inputs) {
   static_assert((std::is_base_of_v<std::istream, IStream> and ...), "Unknown parameters passed to stream()");
   
   auto builder = Builder();
   (builder.append(inputs) and ...);
   return builder.build();
}

} //namespace pipe
