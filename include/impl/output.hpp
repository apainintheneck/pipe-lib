#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "option.hpp"

//
// Classes used to mimick the output behavior after a series of shell commands.
//
namespace pipe {

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
class Tee {
public:
   template <typename option = opt::none>
   Tee(const std::string& filename) {
      data = std::make_shared<Data>();
      add<option>(filename);
   }
   
   Tee(std::ostream& stream) {
      data = std::make_shared<Data>();
      add(stream);
   }
   ~Tee() = default;
   
   //
   // Operators
   //
   
   template <typename T>
   Tee operator<<(const T& t) {
      for(auto& out : data->ostreams)
         out.get() << t;
      
      for(auto& out : data->ofstreams)
         out << t;
      
      return *this;
   }
   
   //
   // Add
   //
   
   template <typename option = opt::none>
   Tee add(const std::string& file) {
      if constexpr(std::is_same_v<option, opt::a>) {
         std::ofstream outfile(file, std::ios::out | std::ios::app);
         if(outfile.is_open())
            data->ofstreams.push_back(std::move(outfile));
      } else {
         std::ofstream outfile(file);
         if(outfile.is_open())
            data->ofstreams.push_back(std::move(outfile));
      }
      
      return *this;
   }
   
   Tee add(std::ostream& stream) {
      data->ostreams.push_back(std::ref(stream));
      
      return *this;
   }

private:
   
   //
   // Data Class
   //
   
   struct Data {
      std::vector<std::reference_wrapper<std::ostream>> ostreams;
      std::vector<std::ofstream> ofstreams;
   };
   
   //
   // Variables
   //
   
   std::shared_ptr<Data> data;
};

} // namespace pipe
