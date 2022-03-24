//
//  Header.h
//  shell-lib
//
//  Created by Kevin on 3/16/22.
//

#pragma once

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <vector>

namespace sh {

using path = std::filesystem::path;

path pwd() noexcept {
   std::error_code err;
   return std::filesystem::current_path(err);
}

bool cd() noexcept {
   std::error_code err;
   std::filesystem::current_path(std::getenv("HOME"), err);
   return !err;
}

bool cd(const path& dest) noexcept {
   std::error_code err;
   std::filesystem::current_path(dest, err);
   return !err;
}

bool mv(const path& src, const path& dest) noexcept {
   std::error_code err;
   std::filesystem::rename(src, dest, err);
   return !err;
}

bool rm(const path& src, const bool recursive = false) noexcept {
   std::error_code err;
   if(recursive) {
      std::filesystem::remove_all(src, err);
      return !err;
   } else {
      return std::filesystem::remove(src, err);
   }
}

bool cp(const path& src, const path& dest, const bool recursive = false) noexcept {
   std::error_code err;
   if(recursive) {
      std::filesystem::copy(src, dest, std::filesystem::copy_options::recursive, err);
      return !err;
   } else {
      return std::filesystem::copy_file(src, dest, err);
   }
}

bool mkdir(const path& dir) noexcept {
   std::error_code err;
   return std::filesystem::create_directory(dir, err);
}

bool rmdir(const path& dir) noexcept {
   if(!std::filesystem::is_directory(dir) || std::filesystem::is_empty(dir))
      return false;
   
   std::error_code err;
   return std::filesystem::remove(dir, err);
}

class ls {
public:
   using dir_iter = std::filesystem::directory_iterator;
   
   ls() = default;
   ls(const path& directory_path) : dir_path_(directory_path) {
      open();
   }
   ls(path&& directory_path) : dir_path_(std::move(directory_path)) {
      open();
   }
   
   bool open(const path& directory_path) {
      dir_path_ = directory_path;
      return open();
   }
   bool open(path&& directory_path) {
      dir_path_ = std::move(directory_path);
      return open();
   }
   
   dir_iter begin() { return begin_; }
   dir_iter end() { return end_; }
   
private:
   bool open() {
      std::error_code err;
      dir_iter it(dir_path_, err);
      
      if(err) {
         is_open_ = false;
         dir_path_.clear();
      } else {
         is_open_ = true;
         begin_ = it;
      }
         
      return is_open_;
   }
   
   dir_iter begin_;
   dir_iter end_;
   
   path dir_path_;
   bool is_open_;
};

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

bool  echo(const char* input) noexcept {
   return std::puts(input) >= 0;
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

//It'd be great to find a better way to alias dirname and basename since they're identical.
std::string dirname(path filepath) {
   filepath.remove_filename();
   return filepath;
}

std::string basename(path filepath) {
   filepath.remove_filename();
   return filepath;
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

std::filesystem::file_type file(const path& filepath) noexcept {
   std::error_code err;
   const auto status = std::filesystem::status(filepath, err);
   return status.type();
}

//Add a more intuitive way to add arguments
bool chmod(const path& filepath, const std::filesystem::perms permissions) noexcept {
   std::error_code err;
   std::filesystem::permissions(filepath, permissions, err);
   return !err;
}

namespace test {

bool b(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_block_file(file, err) && !err;
}

bool c(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_character_file(file, err) && !err;
}

bool d(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_directory(file, err) && !err;
}

bool f(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_regular_file(file, err) && !err;
}

bool p(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_fifo(file, err) && !err;
}

bool L(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_symlink(file, err) && !err;
}

bool S(const path& file) noexcept {
   std::error_code err;
   return std::filesystem::is_socket(file, err) && !err;
}

} //namespace test

//size_t wc(std::istream& input) {
//
//}

std::string logname() {
   return std::getenv("USER");
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

/*
 chown
 cksum
 cmp
 comm
 cut
 diff
 date
 expr
 fold
 find
 getopts
 grep
 join
 kill
 make
 mkfifo
 more
 paste
 read
 sed
 tail
 time
 tr
 umask
 zcat
 ps
 du
 */

} //namespace sh
