#  shell-lib

This library aims to replicate the feel of common shell commands in cpp. It works by wrapping common standard library functions and methods to give the illusion that you are executing commands at the command line.

## Structure

Currently there are two files in shell-lib each under the sh namespace. The first is the sytem.hpp header which includes all the system type information getting commands like ls, rm, and cd. The second is the streams.hpp header which includes all the stream manipulation methods like cat, head, tail, and uniq. Some of these currently only provide one version of the command while others have options by creating an extra namespace. For example, you can access test options by going to sh::test::b for example to see if a file is a block file.

## Pipe Structure
sh::Echo<opt::n>("Hello World!")  >> sh::File("example.txt");

## TODO:
- Add the following commands
      wc
      chown
      cksum
      cmp
      comm
      cut
      diff
      date
      expr
      fold
      find
      getopts
      grep
      join
      kill
      make
      mkfifo
      more
      paste
      read
      sed
      time
      tr
      umask
      zcat
      ps
      du
      pushd
      popd
      dirs
      stat, readlink
      exec
- Update current commands and add additional options
