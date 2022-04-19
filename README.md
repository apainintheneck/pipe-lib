#  pipe-lib

This library aims to replicate the feel of combining popular shell commands to manipulate streams of data.

## Structure

The main file to include is the lib/pipe-lib.hpp header which includes all the top level pipe creators. The structure of the program is pretty simple.

Pipe class = This is the main stream manipulation device that works like a builder meaning you can keep calling methods until you are blue in the face.
Output classes = The File, Tee and ostream classes are used to get data out of the pipe.
Options = These are compile time options used to change the behavior of different shell commands. They are also meant to mimic the feel of passing options to command line programs.
PipeBuilder class = Used to build a new pipe. The pipe doesn't get constructed by itself. It must be built.
Start command methods = Use the builder to create a pipe with the given input and return it so the user can mess around with it. This means that the pipe::cat, pipe::echo, and pipe::stream methods take arguments and feed them to the builder according to the options passed to them. This is helpful because it allows you to use template specialization and provide static asserts to check that the options are valid at compile time.

## Pipe Structure
pipe::Echo<opt::n>("Hello World!")  >> pipe::File("example.txt");

## Note:
While awk and sed options exist in the std::regex library they don't behave the what you'd expect. It would probably be best to leave these two commands out for now unless we can come up with a better user interface.

## TODO:
- Start Commands
find
ls
du
ps
- Filter Commands
cmp
comm
cut
diff
fold
join
paste
tr
- Update current commands and add additional options
