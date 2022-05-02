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

template <typename ...Options>
Pipe cat(std::initializer_list<std::string> filenames) {
   using GivenOptions = opt::list<Options...>;
   using AllowedOptions = opt::list<opt::s, opt::n, opt::b>;
   static_assert(GivenOptions::empty() or AllowedOptions::contains_all<Options...>(),
                 "Unknown option passed to cat()");
   if(filenames.size() == 0) return Builder().build();
   
   auto builder = Builder();
   
   for(const auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   // Options -b and -n are both used to number lines but
   // since -b is more specific (it only operates on non-blank lines
   // it is given precedence here.
   if constexpr(GivenOptions::template contains<opt::b>()) {
      builder.number_non_blank_lines();
   } else if constexpr(GivenOptions::template contains<opt::n>()) {
      builder.number_lines();
   }
   
   // Option -s is for squeezing blank lines and doesn't work
   // with -n because that numbers all the lines.
   if constexpr(not GivenOptions::template contains<opt::n>()
                and GivenOptions::template contains<opt::s>()) {
      builder.squeeze_blank_lines();
   }
   
   return builder.build();
}

template <typename ...Options>
Pipe cat(const std::string& filename) {
   return cat<Options...>(std::initializer_list<std::string>{filename});
}

//
// Echo command
//

Pipe echo(std::initializer_list<std::string> strs) {
   if(strs.size() == 0) return Builder().build();
   
   const auto concat_with_space = [](std::string a, std::string b) { return a + "" + b; };
   auto concat_strs = std::accumulate(strs.begin() + 1, strs.end(), *strs.begin(), concat_with_space);
   std::istringstream input(concat_strs);
   
   auto builder = Builder();
   builder.append(input);
   return builder.build();
}

Pipe echo(const std::string& str) {
   return echo(std::initializer_list<std::string>{str});
}

//
// Stream command
//
template <typename ...IStream>
Pipe stream(IStream& ...inputs) {
   static_assert((std::is_base_of_v<std::istream, IStream> and ...), "Unknown parameters passed to stream()");
   
   auto builder = Builder();
   ((builder.append(inputs)), ...);
   return builder.build();
}

} //namespace pipe
