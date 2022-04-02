//
//  main.cpp
//  shell-lib
//
//  Created by Kevin on 3/16/22.
//

#include "../shell-lib/pipe.hpp"
#include "../shell-lib/system.hpp"
#include <iostream>

int main(int argc, const char * argv[]) {
   std::istringstream in("hellow");
   auto tee = sh::Tee<opt::a>(std::cout, "example.txt", "example2.txt");
   sh::Pipe(in).head() | tee;
}
