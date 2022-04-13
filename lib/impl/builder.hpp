#pragma once

#include "pipe.hpp"

namespace pipe {

class Builder {
public:
   Builder() = default;
   ~Builder() = default;
   
   void append(std::istream& stream);
   
   Pipe build();
   
private:
   Pipe pipe;
};

void Builder::append(std::istream& stream) {
   pipe.append(stream);
}

Pipe Builder::build() {
   return std::move(pipe);
}

} // namespace pipe
