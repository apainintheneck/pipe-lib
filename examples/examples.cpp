//
// Examples for pipe-lib
//
#include "../include/pipe-lib.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   std::istringstream in1("   zello\nhello\nhello\nworld\nworld");
   pipe::stream(in1).tr<opt::s>("l").fold() | std::cout;
   
   std::cout << '\n';
   
   std::istringstream in2("hel&&lo\n&&&HELLO\nhell&&&o\nzorld\n&&world\nWorld");
   pipe::stream(in2).uniq().sort<opt::f, opt::d, opt::s>().grep<opt::i>("world").tr("[:lower:]", "[:upper:]") | std::cout;
}
