//
//  Header.h
//  shell-lib
//
//  Created by Kevin on 3/16/22.
//

#pragma once

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

namespace sh {

using path = std::filesystem::path;

path pwd() noexcept {
   std::error_code err;
   return std::filesystem::current_path( err );
}

bool cd(const path& dest) noexcept {
   std::error_code err;
   std::filesystem::current_path( dest, err );
   return !err;
}

bool mv(const path& src, const path& dest) noexcept {
   std::error_code err;
   std::filesystem::rename( src, dest, err );
   return !err;
}

bool rm(const path& src, const bool recursive = true) noexcept {
   std::error_code err;
   if(recursive) {
      std::filesystem::remove_all( src, err );
      return !err;
   } else {
      return std::filesystem::remove( src, err );
   }
}

bool cp(const path& src, const path& dest, const bool recursive = true) noexcept {
   std::error_code err;
   if(recursive) {
      std::filesystem::copy( src, dest, std::filesystem::copy_options::recursive, err );
      return !err;
   } else {
      return std::filesystem::copy_file( src, dest, err );
   }
}

bool mkdir(const path& dir) noexcept {
   std::error_code err;
   return std::filesystem::create_directory( dir, err );
}

bool rmdir(const path& dir) noexcept {
   if(!std::filesystem::is_directory(dir) || std::filesystem::is_empty(dir))
      return false;
   
   std::error_code err;
   return std::filesystem::remove( dir, err );
}

//Should include recursive version. Maybe should include wrapper class version too so that it can go inside a ranged for loop.
//template <typename container>
//container<dir> ls() {
//
//}
//
//template <typename container>
//container<process> ps() {
//
//}

bool cat(const std::string& filename, std::ostream& os = std::cout) {
   std::ifstream file(filename);
   
   if(file.is_open()) {
      os << file.rdbuf();
      return true;
   }
   
   return false;
}

void echo(const char* input) noexcept {
   std::puts( input );
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
   return std::getenv( input );
}

bool touch(const std::string& filepath) noexcept {
   std::ofstream file( filepath, std::ios::out | std::ios::app );
   return file.is_open();
}

std::string dirname(path filepath) {
   filepath.remove_filename();
   return filepath;
}

std::string basename(const path& filepath) {
   return dirname(filepath);
}

bool uniq(const std::string& filepath, std::ostream& os = std::cout) {
   std::ifstream file(filepath);
   if(!file.is_open())
      return false;
   
   std::string prev;
   if(std::getline(file, prev))
      os << prev << '\n';
   
   std::string buffer;
   while(std::getline(file, buffer)) {
      if(buffer == prev) continue;
      
      os << buffer << '\n';
      prev = std::move(buffer);
   }
   
   return true;
}

//size_t wc(std::istream& input) {
//
//}

/*
 alias
 chmod
 chown
 cksum
 cmp
 comm
 command
 cut
 diff
 date
 expr
 fold
 file
 find
 getopts
 grep
 head
 join
 kill
 logname
 make
 mkfifo
 more
 paste
 read
 sed
 sort
 tail
 test
 time
 tr
 tsort
 umask
 unalias
 who
 zcat
 symlink - don't know what it's called
 */

}
