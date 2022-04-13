#pragma once

#include "option.hpp"

//
// Classes used to mimick the output behavior after a series of shell commands.
//
namespace sh {

/*
 
 File
 
 */

// A simple file class that is used to output the value of a pipe
// by mimicking shell syntax with > and >>.
//
// It can be used to output to a new file or overwrite an existing one.
// Ex. sh::Pipe(is) > sh::File("example.txt");
//
// Or just append to the end of an existing file.
// Ex. sh::Pipe(is) >> sh::File("example.txt");
class File {
public:
   File(const std::string& filename) : _filename(filename) {}
   File(std::string&& filename) : _filename(std::move(filename)) {}
   ~File() = default;

   const std::string _filename;
};

/*
 
 Tee
 
 */

// A replacement for the tee command by writing to an output stream,
// writing or appending to files and mimicking shell syntax with |.
//
// It can be used with one file and an outuput stream (the default is std::cout).
// Ex. sh::Pipe(is) | sh::Tee("example.txt"); // Overwrite example
// Ex. sh::Pipe(is) | sh::Tee(os, "example.txt"); // Explicit ostream example
// Ex. sh::Pipe(is) | sh::Tee<opt::a>("example.txt"); // Append example
//
// Or with multiple files at once.
// Ex. sh::Pipe(is) | sh::Tee("example.txt", "another.txt", "one_more.txt");
template <typename option = opt::none>
class Tee {
public:
   template <typename... Filenames>
   Tee(Filenames... files) : _out(std::cout) {
      (add_ofstream(files), ...);
   }
   template <typename... Filenames>
   Tee(std::ostream& os, Filenames... files) : _out(os) {
      (add_ofstream(files), ...);
   }
   ~Tee() = default;
   
   //
   // Operators
   //
   void operator<<(const std::string& str) {
      _out << str;
      
      for(auto& outfile : _outfiles)
         outfile << str;
   }
   
   void operator<<(const char ch) {
      _out << ch;
      
      for(auto& outfile : _outfiles)
         outfile << ch;
   }

private:
   //
   // Init
   //
   void add_ofstream(const std::string& file) {
      if constexpr(std::is_same_v<option, opt::a>) {
         std::ofstream outfile(file, std::ios::out | std::ios::app);
         if(outfile.is_open())
            _outfiles.push_back(std::move(outfile));
      } else {
         std::ofstream outfile(file);
         if(outfile.is_open())
            _outfiles.push_back(std::move(outfile));
      }
   }
   
   //
   // Variables
   //
   std::ostream& _out;
   std::vector<std::ofstream> _outfiles;
};

} // namespace sh
