# ksv

Yet another CSV or TSV library for C.

## System Requirements

* A C compiler that supports ANSI C
* GNU Make
* Valgrind (only for tests on GNU/Linux)

To use GNU Make on non-Linux Unix or Unix-like systems, use `gmake` instead of `make`. To use GNU Make on Windows, use `mingw32-make`.

We compile and run **ksv** with GCC, Clang, Visual C++ and Intel C++ Compiler.

We test **ksv** against several Unix or Unix-like systems:

* Ubuntu 18.04 LTS
* CentOS 8
* openSUSE Leap 15.1
* TrueOS, which is FreeBSD-compatible
* Solaris 11

In addition, we test **ksv** against Windows 10 as well.

## Usage of the Library

Pending.

## Usage of the Console Tool

Pending.

## Known Issues or Bugs

* C file streams may not work in C++ binding
* Dynamic libraries compiled by MSVC still crash

## Note

Currently, **ksv** is distributed mainly as a C library. The **ksv** cli tool is just a byproduct of the library, never a feature-rich CSV or TSV tool.

We copy some utility code from [cwchentw/clibs](https://github.com/cwchentw/clibs).

## Copyright

Copyright (c) 2020 Michael Chen. Licensed under MIT.
