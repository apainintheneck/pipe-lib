#  pipe-lib

This library aims to replicate the feeling of combining popular shell commands to manipulate streams of data. It does this by creating a fluent interface that allows you to chain shell commands.

## Structure

This project is header only so the lib/pipe-lib/ needs to be dropped in a convenient place in your project and pipe-lib/pipe-lib.hpp needs to be included in a source file (look at examples/ for more details). The structure of the program is pretty simple.

Pipe class = This is the main stream manipulation device that allows you to chain commands.
Output classes = The File, Tee, std::ostream, and std::string classes are used to get data out of the pipe.
Options = These are compile time options used to change the behavior of different shell commands. They are also meant to mimic the feel of passing options to command line programs.
Builder class = Used to build a new pipe. The pipe doesn't get constructed by itself. It must be built.
Start command methods = Use the builder to create a pipe with the given input and return it so the user can mess around with it. This means that the pipe::cat, pipe::echo, and pipe::stream methods take arguments and feed them to the builder according to the options passed to them. This is helpful because it allows you to use template specialization and provide static asserts to check that the options are valid at compile time.

## Pipe Structure
pipe::echo<opt::n>("Hello World!")  >> pipe::File("example.txt");

## Note:
While awk and sed options exist in the std::regex library they don't behave the what you'd expect. It would probably be best to leave these two commands out for now unless we can come up with a better user interface.

## TODO:
sed
cut
join
paste
- Update current commands and add additional options
