#  pipe-lib

This library aims to replicate the feeling of combining popular shell commands to manipulate streams of data. It does this by creating a fluent interface that allows you to chain shell commands. This is exploratory and not made for real world development.

---

## Overview

---

## Structure

This project is header only so the `include/pipe-lib/` needs to be dropped in a convenient place in your project and `pipe-lib/pipe-lib.hpp` needs to be included in a source file (look at `examples/` for more details). The structure of the program is pretty simple.

- Pipe class
  - This is the main stream manipulation device that allows you to chain commands.
- Output classes
  - The `pipe::File`, `pipe::Tee`, `std::ostream`, and `std::string` classes are used to get data out of the pipe.
- Options 
  - These are compile time options used to change the behavior of different shell commands. They are also meant to mimic the feel of passing options to command line programs.
- Builder class
  - Used to build a new pipe. The pipe doesn't get constructed by itself. It must be built.
- Start command methods
  - Use the builder to create a pipe with the given input and return it so the user can mess around with it. This means that the `pipe::cat`, `pipe::echo`, and `pipe::stream` methods take arguments and feed them to the builder according to the options passed to them. This is helpful because it allows you to use template specialization and provide static asserts to check that the options are valid at compile time.

## Features

### Open
Pipes are opened using the `cat`, `echo` or `stream` commands.

```c++
pipe::cat("directory/filename.txt") // Reads from file
pipe::echo("example text")          // Reads from string
pipe::stream(istream)               // Reads from istream

pipe::cat({"file1.txt, file2.txt, file3.txt"}) // Reads from multiple files
pipe::echo({"one", "two", "three"})            // Reads from multiple strings
pipe::stream({stream1, stream2, stream3})      // Reads from multiple istreams
```

These commands are not usually used by themselves but allow us to then chain different pipe manipulation commands. Each can also take multiple arguments with initializer lists.

```c++
auto var = pipe::stream(istream);
```

Pipes are classes so they can also be assigned to variables and combined with other pipes in a few nifty ways which will be shown below.

In review, this is where the pipelining starts.

### Close

To close pipes you must redirect the data of the pipe object to one of the data objects. Once data leaves the pipe the pipe will be empty meaning you can't use it anymore.

```c++
// File class
pipe::stream(istream) > pipe::File("example.txt");
pipe::stream(istream) >> pipe::File("example.txt");
```

The `file` class allow you to write the data directly to a file. The `>` allows you to overwrite the file and the `>>` allows you to append to a file just like in Bash.

```c++
// Tee class
pipe::stream(istream) | pipe::Tee(std::cout).add("out.txt");
pipe::stream(istream) | pipe::Tee("out.txt").add("dup.txt");

auto tee = pipe::Tee(std::cout).add("out1.txt").add("out2.txt");
pipe::stream(istream) | tee;
```

The `tee` class allows you to output data to multiple ostreams. It requires one ostream or filename string and then other ostreams and filename strings can be added by using the `Tee.add()`.

```c++
// String
std::string output;

pipe::stream(istream) > output;
pipe::stream(istream) >> output;

// Ostream
std::ostream os;

pipe::stream(istream) | os;
```

There is no need to use the `tee` or `file` classes to retrieve data from a pipe. Pipe data can be written to or appended to a string or piped to an ostream.

### Shell Commands

To manipulate data there are a bunch of different methods which mimic popular shell commands.

```c++
// Command List
pipe::stream(istream).fold(length);
pipe::stream(istream).grep(pattern);
pipe::stream(istream).head(count);
pipe::stream(istream).paste(pipe);
pipe::stream(istream).sort();
pipe::stream(istream).tail(count);
pipe::stream(istream).tr(pattern1, pattern2);
pipe::stream(istream).uniq();
```

### Options

To further mimic the versatility of shell commands you are also able to supply options to some of these commands.

```c++
// Example
pipe::cat<opt::b, opt::s>("in.txt").grep<opt::i, opt::E>("search-term") | std::cout;
```

## Note:
While awk and sed options exist in the std::regex library they don't behave the way you'd expect. It would probably be best to leave these two commands out for now unless we can come up with a better user interface.

## TODO:
- sed
- cut
- join
- paste
- Update current commands and add additional options
- combine cat and stream commands because they essentially do the same thing
