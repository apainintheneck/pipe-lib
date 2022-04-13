#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include "impl/basic_pipe.hpp"

namespace sh {

//
// Cat command
//
template <typename option = opt::none>
class Cat : public detail::BasicPipe {
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

//
// Echo command
//
template <typename option = opt::none>
class Echo : public detail::BasicPipe {
public:
   template <typename... Strings>
   Echo(Strings... strings) {
      std::string joined_strings;
      if constexpr(std::is_same_v<option, opt::n>) {
         joined_strings = (std::string(strings) + ...);
      } else {
         joined_strings = (add_newline(strings) + ...);
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

//
// Pipe command
//
class Pipe : public detail::BasicPipe {
public:
   Pipe(std::istream& input) {
      init(input);
   }
};

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

} //namespace sh
