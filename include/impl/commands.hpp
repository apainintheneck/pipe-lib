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

// TODO: Add -s option that squeezes empty lines. This should probably get added as
// a Pipe helper and then added to Builder as well.
template <typename option = opt::none>
Pipe cat(std::initializer_list<std::string> filenames) {
   using AllowedOptions = opt::list<opt::none, opt::n, opt::b>;
   static_assert(AllowedOptions::contains<option>(), "Unknown option passed to cat()");
   if(filenames.size() == 0) return Builder().build();
   
   auto builder = Builder();
   
   for(const auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   if constexpr(std::is_same_v<option, opt::n>) {
      builder.number_lines();
   } else if constexpr(std::is_same_v<option, opt::b>) {
      builder.number_non_blank_lines();
   }
   
   return builder.build();
}

template <typename option = opt::none>
Pipe cat(const std::string& filename) {
   return cat<option>(std::initializer_list<std::string>{filename});
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
