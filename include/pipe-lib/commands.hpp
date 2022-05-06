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
Pipe cat(std::initializer_list<std::string_view> filenames) {
   using OptionList = opt::list<Options...>;
   static_assert(OptionList::template allows<opt::s, opt::n, opt::b>(),
                 "Unknown option(s) passed to cat()");
   if(filenames.size() == 0) return Builder().build();
   
   auto builder = Builder();
   
   for(auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   // Options -b and -n are both used to number lines but
   // since -b is more specific (it only operates on non-blank lines
   // it is given precedence here.
   if constexpr(OptionList::template contains<opt::b>()) {
      builder.number_non_blank_lines();
   } else if constexpr(OptionList::template contains<opt::n>()) {
      builder.number_lines();
   }
   
   // Option -s is for squeezing blank lines and doesn't work
   // with -n because that numbers all the lines.
   if constexpr(not OptionList::template contains<opt::n>()
                and OptionList::template contains<opt::s>()) {
      builder.squeeze_blank_lines();
   }
   
   return builder.build();
}

template <typename ...Options>
Pipe cat(std::string_view filename) {
   return cat<Options...>({filename});
}

//
// Echo command
//

Pipe echo(std::initializer_list<std::reference_wrapper<const std::string>> strs) {
   if(strs.size() == 0) return Builder().build();
   
   const auto concat_with_space = [](const std::string& a, std::reference_wrapper<const std::string>& b) { return a + " " + b.get(); };
   auto concat_strs = std::accumulate(strs.begin() + 1, strs.end(), strs.begin()->get(), concat_with_space);
   std::istringstream input(concat_strs);
   
   auto builder = Builder();
   builder.append(input);
   return builder.build();
}

Pipe echo(const std::string& str) {
   return echo({std::cref(str)});
}

//
// Stream command
//

Pipe stream(std::initializer_list<std::reference_wrapper<std::istream>> inputs) {
   auto builder = Builder();
   
   for(auto& input : inputs) {
      builder.append(input.get());
   }
   
   return builder.build();
}

Pipe stream(std::istream& input) {
   return stream({std::ref(input)});
}

} //namespace pipe
