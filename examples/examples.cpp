//
// Examples for pipe-lib
//
#include "../lib/pipe-lib.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   std::istringstream in("hello\nhello\nhello\nworld\nworld");
   pipe::stream(in).uniq<opt::c>() | std::cout;
}
