# liboqs-cpp

## C++ bindings for liboqs

**Build status:**

[![Build Status](https://api.travis-ci.com/open-quantum-safe/liboqs-cpp.svg?branch=master)](https://travis-ci.com/open-quantum-safe/liboqs-cpp)

[![Build status](https://ci.appveyor.com/api/projects/status/1xjgtc25bpujm8w9?svg=true)](https://ci.appveyor.com/project/dstebila/liboqs-cpp)

---

**liboqs-cpp** offers a C++ wrapper for the master branch of [Open Quantum Safe](https://openquantumsafe.org/) [liboqs](https://github.com/open-quantum-safe/liboqs/) C library. The wrapper is written in standard C++11, hence in the following it is assumed that you have access to a C++11 compliant complier. liboqs-cpp has been extensively tested on Linux, macOS and Windows systems. Continuous integration is provided via Travis CI and AppVeyor.

## Pre-requisites

liboqs-cpp depends on the [liboqs](https://github.com/open-quantum-safe/liboqs) C library; liboqs master branch must first be compiled as a Linux/macOS/Windows library, see the specific platform building instructions below.

Repository contents
--------

liboqs-cpp is a header-only wrapper. The project contains the following files
and folders:

 - **`VisualStudio/`**`liboqs-cpp.sln`: Visual Studio 2017 solution
 - **`doc`**: Doxygen-generated detailed documentation
 - **`examples/`**`kem.cpp`: key encapsulation example
 - **`examples/`**`sig.cpp`: signature example
 - **`include/oqs_cpp.h`: main header file for the wrapper**
 - **`unit_tests`**: unit tests written using Google Test (included)

Usage
-----

To avoid namespace pollution, liboqs-cpp includes all of its code inside the namespace `oqs`. All of liboqs pure C API is located
in the namespace `oqs::C`, hence to use directly a C API function you must qualify the call with `oqs::C::liboqs_C_function(...)`. 

liboqs-cpp defines four main classes: `oqs::KeyEncapsulation` and `oqs::Signature`, providing post-quantum key encapsulation and signture mechanisms, respectively, and 
`oqs::KEMs` and `oqs::Sigs`, containing only static member functions that provide information related to the available key encapsulation mechanisms or signature mechanism, respectively. 

`oqs::KeyEncapsulation` and/or `oqs::Signature` must be instantiated with a string identifying one of mechanisms supported by liboqs; these can be enumerated using the `oqs::KEMs::get_enabled_KEM_mechanisms()` and `oqs::Sigs::get_enabled_sig_mechanisms()` member functions. 

The wrapper also defines a high resolution timing class, `oqs::Timer<>`.

The examples in the [`examples`](https://github.com/open-quantum-safe/liboqs-cpp/tree/master/examples) folder are self-explanatory and provide more details about the wrapper's API.

Building on POSIX (Linux/UNIX-like) platforms
--------------------------------------------

First you must build the master branch of liboqs according to the [liboqs building instructions](https://github.com/open-quantum-safe/liboqs#building), followed by a `[sudo] make install` to ensure that the compiled library is system-wide visible (by default it installs under `/usr/local/include` and `/usr/local/lib`). Alternatively, you may modify `LIBOQS_INCLUDE_DIR` and `LIBOQS_LIB_DIR` in [`CMakeLists.txt`](https://github.com/open-quantum-safe/liboqs-cpp/blob/master/CMakeLists.txt#L13) so that they point to the location of liboqs headers/library.

Next, to use the wrapper, you simply `#include "oqs_cpp.h"` in your program. The wrapper contains
a CMake build system for both examples and unit tests. To compile and run the examples, create a `build` folder inside the root folder of the project, change
directory to `build`, then type 

`cmake ..; make -j4`

The above commands build all examples in `examples`, i.e. `examples/kem` and `examples/sig`, assuming
the CMake build system is available on your platform.
Replace the `-j4` flag with your
processor's number of cores, e.g. use `-j8` if your system has 8 cores.
To build only a specific example, e.g. `examples/kem`, specify the target as the argument of the `make` command, such as

`make kem`

To compile and run the unit tests, first `cd unit_tests`, then create a `build` folder inside `unit_tests`, change directory to it, and finally type

`cmake ..; make -j4`

The above commands build `tests/oqs_cpp_testing` suite of unit tests.


Building on Windows
--------------------------------

A Visual Studio 2017 solution containing both key encapsulation and signature examples from [`examples`](https://github.com/open-quantum-safe/liboqs-cpp/tree/master/examples) as two separate projects is provided in the [`VisualStudio`](https://github.com/open-quantum-safe/liboqs-cpp/tree/master/VisualStudio) folder. Building instructions:

- First, you must clone/download and build liboqs under Windows, see [liboqs Windows building instructions](https://github.com/open-quantum-safe/liboqs#building-and-running-on-windows) for more details.
- Next, you must [set the environment variable](https://stackoverflow.com/a/32463213/3093378) `LIBOQS_INSTALL_PATH` to point to the location of liboqs, e.g. `C:\liboqs`. 
- Only after completing the steps above you may build the liboqs-cpp solution (or each individual projects within the solution). In case you end up with a linker error, make sure that the corresponding liboqs target was built, i.e. if building a `Release` version with an `x64` target, then the corresponding `Release/x64` solution from liboqs should have been built in advance.

In case you get a "Missing Windows SDK" error, right-click on the solution name and choose "Retarget solution" to re-target the projects in the solution to your available Windows SDK.


Limitations and security
------------------------

liboqs is designed for prototyping and evaluating quantum-resistant cryptography. Security of proposed quantum-resistant algorithms may rapidly change as research advances, and may ultimately be completely insecure against either classical or quantum computers.

We believe that the NIST Post-Quantum Cryptography standardization project is currently the best avenue to identifying potentially quantum-resistant algorithms. liboqs does not intend to "pick winners", and we strongly recommend that applications and protocols rely on the outcomes of the NIST standardization project when deploying post-quantum cryptography.

We acknowledge that some parties may want to begin deploying post-quantum cryptography prior to the conclusion of the NIST standardization project. We strongly recommend that any attempts to do make use of so-called **hybrid cryptography**, in which post-quantum public-key algorithms are used alongside traditional public key algorithms (like RSA or elliptic curves) so that the solution is at least no less secure than existing traditional cryptography.

Just like liboqs, liboqs-cpp is provided "as is", without warranty of any kind. See [LICENSE](https://github.com/open-quantum-safe/liboqs-cpp/blob/master/LICENSE) for the full disclaimer.

License
-------

liboqs-cpp is licensed under the MIT License; see [LICENSE](https://github.com/open-quantum-safe/liboqs-cpp/blob/master/LICENSE) for details.

Team
----

The Open Quantum Safe project is led by [Douglas Stebila](https://www.douglas.stebila.ca/research/) and [Michele Mosca](http://faculty.iqc.uwaterloo.ca/mmosca/) at the University of Waterloo.

liboqs-cpp was developed by [Vlad Gheorghiu](http://vsoftco.github.io) at evolutionQ and University of Waterloo.
