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

void echo(const char* input) noexcept {
   std::puts( input );
}

class tee {
public:
   tee(std::ostream& out1, std::ostream& out2) : out1_(out1), out2_(out2) {}
   
   template <typename T>
   tee& operator<<(T value) {
      out1_ << value;
      out2_ << value;
      
      return *this;
   }
   
private:
   std::ostream& out1_;
   std::ostream& out2_;
};

std::string env(const char* input) noexcept {
   return std::getenv( input );
}

bool touch(const std::string& filepath) noexcept {
   std::ofstream file( filepath, std::ios::out | std::ios::app );
   return file.operator bool();
}

//size_t wc(const std::istream& input) {
//
//}

/*
 alias
 cat
 chmod
 chown
 cksum
 cmp
 comm
 command
 cut
 diff
 date
 dirname
 basename
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
 tty
 tsort
 umask
 unalias
 uname
 uniq
 who
 zcat
 */

}
