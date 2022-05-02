#pragma once

#include <iostream>

#include "pipe.hpp"

namespace pipe {

/*
 
 Builder
 
 */

// A builder for the Pipe class. It makes it so
// new facades can be created to give the illusion
// of first calling other commands at the command line.
//
// For example:
//    pipe::cat()
//    pipe::echo()
//
// These methods really just use the builder to return
// a new, fully-loaded instance of the Pipe class.
class Builder {
public:
   Builder() = default;
   ~Builder() = default;
   
   void append(std::istream& stream);
   
   void number_lines();
   
   void number_non_blank_lines();
   
   void squeeze_blank_lines();
   
   Pipe build();
   
private:
   Pipe pipe;
};

void Builder::append(std::istream& stream) {
   pipe.append(stream);
}

void Builder::number_lines() {
   pipe.number_lines();
}

void Builder::number_non_blank_lines() {
   pipe.number_non_blank_lines();
}

void Builder::squeeze_blank_lines() {
   pipe.squeeze_blank_lines();
}

Pipe Builder::build() {
   return std::move(pipe);
}

} // namespace pipe
