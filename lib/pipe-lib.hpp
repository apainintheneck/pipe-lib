#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>

#include "impl/builder.hpp"

namespace pipe {

//
// Cat command
//
Pipe cat(std::initializer_list<std::string> filenames) {
   auto builder = Builder();
   
   for(const auto& filename : filenames) {
      std::ifstream file(filename);
      
      if(file.is_open())
         builder.append(file);
   }
   
   return builder.build();
}

//
// Echo command
//
template <typename option = opt::none>
Pipe echo(const std::string& str) {
   using AllowedOptions = opt::list<opt::none, opt::n>;
   static_assert(AllowedOptions::contains<option>(), "Unknown option passed to echo()");
   
   std::istringstream input(str);
   
   auto builder = Builder();
   builder.append(input);
   return builder.build();
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
   static_assert((std::is_base_of_v<std::istream, IStream> && ...), "Expected istream& parameters to be passed to cat()");
   
   auto builder = Builder();
   (builder.append(inputs) && ...);
   return builder.build();
}

//
//class ls {
//public:
//   using dir_iter = std::filesystem::directory_iterator;
//   
//   ls() = default;
//   ls(const path& directory_path) : dir_path_(directory_path) {
//      open();
//   }
//   ls(path&& directory_path) : dir_path_(std::move(directory_path)) {
//      open();
//   }
//   
//   bool open(const path& directory_path) {
//      dir_path_ = directory_path;
//      return open();
//   }
//   bool open(path&& directory_path) {
//      dir_path_ = std::move(directory_path);
//      return open();
//   }
//   
//   dir_iter begin() { return begin_; }
//   dir_iter end() { return end_; }
//   
//private:
//   bool open() {
//      std::error_code err;
//      dir_iter it(dir_path_, err);
//      
//      if(err) {
//         is_open_ = false;
//         dir_path_.clear();
//      } else {
//         is_open_ = true;
//         begin_ = it;
//      }
//         
//      return is_open_;
//   }
//   
//   dir_iter begin_;
//   dir_iter end_;
//   
//   path dir_path_;
//   bool is_open_;
//};

} //namespace pipe
