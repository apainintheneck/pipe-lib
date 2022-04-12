//
// Examples for pipe-lib
//
#include "../lib/pipe-lib.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   std::istringstream in("hellow");
   auto tee = sh::Tee<opt::a>(std::cout, "example.txt", "example2.txt");
   sh::Pipe(in).head<opt::n>(20).wc<opt::l>() | tee;
}
