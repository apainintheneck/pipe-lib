//
// Examples for pipe-lib
//
#include "../lib/pipe-lib.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   std::istringstream in1("   zello\nhello\nhello\nworld\nworld");
   pipe::stream(in1).uniq().sort<opt::b, opt::r>() | std::cout;
   
   std::cout << '\n';
   
   std::istringstream in2("hel&&lo\n&&&HELLO\nhell&&&o\nzorld\n&&world\nworld");
   pipe::stream(in2).uniq().sort<opt::f, opt::d, opt::s>() | std::cout;
}
