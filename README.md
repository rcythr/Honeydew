Note: For a prettier version of this README please see the project page : http://rcythr.github.io/Honeydew

#General

**Honeydew** is a project that is dedicated to providing powerful concurrency primitives with a clean interface. With **Honeydew** you can quickly model relationships between various tasks that are performed throughout your system. This allows you to stop worrying about how to coordinate events and start worrying about the implementation of your business logic.

__Version:__ 0.8.2

__Release date:__ 2014.03.10

__Project state:__ stable

__Author:__ Richard W. Laughlin Jr. (@rcythr)

# Usage

See the project wiki for information regarding usage.

# Dependencies:

* **C++11 Standard Compliant Compiler** (Clang 3.1, GCC 4.8, Etc). Older versions than the above *may* work depending on compliance.
* **CMake 2.8** (for building from source)

# Installation Instructions

To simply build the library and include directories into a build folder, from command line:

    mkdir build
    cd build
    cmake ..
    make install

The library will be in [PROJECT ROOT]/build/lib

The includes will be in [PROJECT ROOT]/build/include/

The test executables will be in [PROJECT ROOT]/build/bin

# Documentation

Documentation may be found on the project wiki and in the associated **.hpp** files and in code.

# Changelog

+ **0.8.2** - Event System Helpers. CMake. Lots of stuff.
+ **0.6.0** - Pipeline implementation.
+ **0.5.0** - Basic implementation is complete with continuations and joins.
