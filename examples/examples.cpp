//
// Examples for pipe-lib
//
#include "../include/pipe-lib.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   pipe::cat<opt::n>("alice-in-wonderland.txt").grep("Alice").tail() | std::cout;
   
   std::cout << '\n';
   
   pipe::echo("   zello\nhello\nhello\nworld\nworld").tr<opt::s>("l").fold() | std::cout;
   
   std::cout << '\n';
   
   std::istringstream in2("hel&&lo\n&&&HELLO\nhell&&&o\nzorld\n&&world\nWorld");
   pipe::stream(in2).uniq().sort<opt::f, opt::d, opt::s>().grep<opt::i>("world").tr("[:lower:]", "[:upper:]") | std::cout;
}
