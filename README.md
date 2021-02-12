liboqs-cpp: C++ bindings for liboqs
===================================

[![Build status - CircleCI Linux/macOS](https://circleci.com/gh/open-quantum-safe/liboqs-cpp.svg?style=svg)](https://circleci.com/gh/open-quantum-safe/liboqs-cpp)
[![Build status - Windows](https://ci.appveyor.com/api/projects/status/v7b5ner85txm8u77?svg=true)](https://ci.appveyor.com/project/vsoftco/liboqs-cpp)

---

**liboqs-cpp** offers a C++ wrapper for
the [Open Quantum Safe](https://openquantumsafe.org/) [liboqs](https://github.com/open-quantum-safe/liboqs/)
C library, which is a C library for quantum-resistant cryptographic algorithms.

The wrapper is written in standard C++11, hence in the following it is assumed
that you have access to a C++11 compliant complier. liboqs-cpp has been
extensively tested on Linux, macOS and Windows platforms. Continuous integration
is provided via CircleCI and AppVeyor.

## Pre-requisites

liboqs-cpp depends on the [liboqs](https://github.com/open-quantum-safe/liboqs)
C library; liboqs must first be compiled as a Linux/macOS/Windows library (i.e.
using `ninja install` with `-DBUILD_SHARED_LIBS=ON` during configuration), see
the specific platform building instructions below.

Contents
--------

liboqs-cpp is a header-only wrapper. The project contains the following files
and directories:

- **`include/oqs_cpp.h`: main header file for the wrapper**
- `include/rand/rand.h`: support for RNGs from `<oqs/rand.h>`
- `examples/kem.cpp`: key encapsulation example
- `examples/rand.cpp`: RNG example
- `examples/sig.cpp`: signature example
- `unit_tests`: unit tests written using Google Test (included)

Usage
-----

To avoid namespace pollution, liboqs-cpp includes all of its code inside the
namespace `oqs`. All of the liboqs C API is located in the namespace `oqs::C`,
hence to use directly a C API function you must qualify the call
with `oqs::C::liboqs_C_function(...)`.

liboqs-cpp defines four main classes: `oqs::KeyEncapsulation`
and `oqs::Signature`, providing post-quantum key encapsulation and signture
mechanisms, respectively, and
`oqs::KEMs` and `oqs::Sigs`, containing only static member functions that
provide information related to the available key encapsulation mechanisms or
signature mechanism, respectively.

`oqs::KeyEncapsulation` and/or `oqs::Signature` must be instantiated with a
string identifying one of mechanisms supported by liboqs; these can be
enumerated using the `oqs::KEMs::get_enabled_KEM_mechanisms()`
and `oqs::Sigs::get_enabled_sig_mechanisms()` member functions.

Support for alternative RNGs is provided by the `include/rand/rand.h` header
file, which exports its functions in `namespace oqs::rand`. This header file
must be explicitly included in order to activate the support for alternative
RNGs.

The wrapper also defines a high resolution timing class, `oqs::Timer<>`.

The examples in
the [`examples`](https://github.com/open-quantum-safe/liboqs-cpp/tree/main/examples)
directory are self-explanatory and provide more details about the wrapper's API.

Building on POSIX (Linux/UNIX-like) platforms
---------------------------------------------

First, you must build liboqs according to
the [liboqs building instructions](https://github.com/open-quantum-safe/liboqs#linuxmacos)
with shared library support enabled (add `-DBUILD_SHARED_LIBS=ON` to the `cmake`
command), followed (optionally) by a `sudo ninja install` to ensure that the
shared library is visible system-wide (by default it installs
under `/usr/local/include` and `/usr/local/lib` on Linux/macOS).

You may need to set the `LD_LIBRARY_PATH` (`DYLD_LIBRARY_PATH` on macOS)
environment variable to point to the path to liboqs' library directory, e.g.

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

assuming `liboqs.so.*` were installed in `/usr/local/lib` (true if you
ran `sudo ninja install` after building liboqs).

Next, to use the wrapper, you simply `#include "oqs_cpp.h"` in your program. The
wrapper contains a CMake build system for both examples and unit tests. To
compile and run the examples, create a `build` directory inside the root
directory of the project, change directory to `build`, then type

	cmake .. -DLIBOQS_INCLUDE_DIR=/usr/local/include -DLIBOQS_LIB_DIR=/usr/local/lib
	make -j4

The above commands build all examples in `examples`, together with the unit
tests suite, assuming the CMake build system is available on your platform.
The `-DLIBOQS_INCLUDE_DIR` and `-DLIBOQS_LIB_DIR` flags specify the location to
the liboqs headers and compiled library, respectively. You may omit those flags
and simply type `cmake .. && make -j4` in case you installed liboqs
in `/usr/local` (true if you ran `sudo ninja install` after building liboqs).
You may replace the `-j4` flag with your processor's number of cores, e.g.
use `-j8` if your system has 8 cores. To build only a specific example,
e.g. `examples/kem`, specify the target as the argument of the `make` command,
such as

	make kem

To run the unit tests, type

	make test # or ctest 

after building the project. Use `GTEST_COLOR=1 ARGS="-V" make test` or
`GTEST_COLOR=1 ctest -V` for coloured verbose testing output.

Building on Windows
-------------------

We provide CMake support for Visual Studio. We recommend using Visual Studio
2017 or later (preferably Visual Studio 2019). For comprehensive details about
using CMake with Visual Studio please
read [this page](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)
.

Ensure that the liboqs shared library `oqs.dll` is visible system-wide. Use
the "Edit the system environment variables" Control Panel tool or type in a
Command Prompt

	set PATH="%PATH%;C:\some\dir\liboqs\build\bin"

of course replacing the paths with the ones corresponding to your system.

Documentation
-------------
To generate the full official API documentation in both PDF and HTML formats run
[`doxygen`](http://www.doxygen.nl) on
the [`Doxyfile`](https://github.com/open-quantum-safe/liboqs-cpp/blob/main/Doxyfile)
file. The tool `dot` from the [`Graphviz`](https://www.graphviz.org) package
must be installed (`sudo apt-get install graphviz` in Ubuntu/Debian).
Running `doxygen` will generate the documentation directory `doc` containing
both the HTML and LaTeX documentation.

The HTML documentation file will be accessible by opening `doc/html/index.html`
with the browser of your choice.

To generate a PDF file of the documentation, run

```bash
latexmk -pdf refman.tex
```

from the `doc/latex` directory or compile the file `doc/latex/refman.tex` with
your LaTeX compiler. This will create the `doc/latex/refman.pdf` documentation
file. Consult your favourite LaTeX manual for how to compile/build LaTeX files
under your specific operating system.

Limitations and security
------------------------

liboqs is designed for prototyping and evaluating quantum-resistant
cryptography. Security of proposed quantum-resistant algorithms may rapidly
change as research advances, and may ultimately be completely insecure against
either classical or quantum computers.

We believe that the NIST Post-Quantum Cryptography standardization project is
currently the best avenue to identifying potentially quantum-resistant
algorithms. liboqs does not intend to "pick winners", and we strongly recommend
that applications and protocols rely on the outcomes of the NIST standardization
project when deploying post-quantum cryptography.

We acknowledge that some parties may want to begin deploying post-quantum
cryptography prior to the conclusion of the NIST standardization project. We
strongly recommend that any attempts to do make use of so-called **hybrid
cryptography**, in which post-quantum public-key algorithms are used alongside
traditional public key algorithms (like RSA or elliptic curves) so that the
solution is at least no less secure than existing traditional cryptography.

Just like liboqs, liboqs-cpp is provided "as is", without warranty of any kind.
See [LICENSE](https://github.com/open-quantum-safe/liboqs-cpp/blob/main/LICENSE)
for the full disclaimer.

License
-------

liboqs-cpp is licensed under the MIT License;
see [LICENSE](https://github.com/open-quantum-safe/liboqs-cpp/blob/main/LICENSE)
for details.

Team
----

The Open Quantum Safe project is led
by [Douglas Stebila](https://www.douglas.stebila.ca/research/)
and [Michele Mosca](http://faculty.iqc.uwaterloo.ca/mmosca/) at the University
of Waterloo.

liboqs-cpp was developed by [Vlad Gheorghiu](http://vsoftco.github.io) at
evolutionQ and University of Waterloo.

### Support

Financial support for the development of Open Quantum Safe has been provided by
Amazon Web Services and the Canadian Centre for Cyber Security.

We'd like to make a special acknowledgement to the companies who have dedicated
programmer time to contribute source code to OQS, including Amazon Web Services,
evolutionQ, and Microsoft Research.

Research projects which developed specific components of OQS have been supported
by various research grants, including funding from the Natural Sciences and
Engineering Research Council of Canada (NSERC); see the source papers for
funding acknowledgments.
