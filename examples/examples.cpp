//
// Examples for pipe-lib
//
#include "../include/pipe-lib.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   auto tee = pipe::Tee(std::cout).add("out.txt");
   pipe::cat<opt::n>("alice-in-wonderland.txt").grep("Alice").tail() | tee;
   
   std::cout << '\n';
   
   pipe::cat<opt::b, opt::s>("in.txt") | std::cout;
   
   std::cout << '\n';

   tee << "Hello World!\n" << '\n';
   
   std::cout << '\n';

   pipe::echo("   zello\n\n\n\n\n\nhello\nhello\nworld\nworld").tr<opt::s>("l").fold() | std::cout;

   std::cout << '\n';

   std::istringstream in1("hel&&lo\n&&&HELLO\n&&&HELLO\nhell&&&o\nzorld\n&&world\nWorld");
   std::istringstream in2("next");
   pipe::stream(in1, in2).uniq().tr("[:lower:]", "[:upper:]") | std::cout;
}
