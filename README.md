# liboqs-cpp: C++ bindings for liboqs

[![GitHub actions](https://github.com/open-quantum-safe/liboqs-cpp/actions/workflows/cmake.yml/badge.svg)](https://github.com/open-quantum-safe/liboqs-cpp/actions)

---

## About

The **Open Quantum Safe (OQS) project** has the goal of developing and
prototyping quantum-resistant cryptography.

**liboqs-cpp** offers a C++ wrapper for
the [Open Quantum Safe](https://openquantumsafe.org/) [liboqs](https://github.com/open-quantum-safe/liboqs/)
C library, which is a C library for quantum-resistant cryptographic algorithms.

The wrapper is written in standard C++11, hence in the following it is assumed
that you have access to a C++11 compliant compiler. liboqs-cpp has been
extensively tested on Linux, macOS and Windows platforms. Continuous integration
is provided via GitHub actions.

The project contains the following files and directories:

- **`include/oqs_cpp.hpp`: main header file for the wrapper**
- `include/common.hpp`: utility code
- `include/rand/rand.hpp`: support for RNGs from `<oqs/rand.h>`
- `examples/kem.cpp`: key encapsulation example
- `examples/rand.cpp`: RNG example
- `examples/sig.cpp`: signature example
- `unit_tests`: unit tests written using GoogleTest

---

## Pre-requisites

- [liboqs](https://github.com/open-quantum-safe/liboqs)
- [git](https://git-scm.com/)
- [CMake](https://cmake.org/)
- C++11 compliant compiler, e.g., [gcc](https://gcc.gnu.org/)
  , [clang](https://clang.llvm.org)
  , [MSVC](https://visualstudio.microsoft.com/vs/) etc.

---

## Installation

### Configure, build and install liboqs

Execute in a Terminal/Console/Administrator Command Prompt

```shell
git clone --depth=1 https://github.com/open-quantum-safe/liboqs
cmake -S liboqs -B liboqs/build -DBUILD_SHARED_LIBS=ON
cmake --build liboqs/build --parallel 8
cmake --build liboqs/build --target install
```

The last line may require prefixing it by `sudo` on UNIX-like systems.
Change `--parallel 8` to match the number of available cores on your system.

On UNIX-like platforms, you may need to set
the `LD_LIBRARY_PATH` (`DYLD_LIBRARY_PATH` on macOS) environment variable to
point to the path to liboqs' library directory, e.g.,

```shell
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

On Windows platforms, **you must ensure** that the liboqs shared
library `oqs.dll` is visible system-wide, and that the following environment
variable are being set. Use the "Edit the system environment variables" Control
Panel tool or execute in a Command Prompt, e.g.,

```shell
set PATH=%PATH%;C:\Program Files (x86)\liboqs\bin
```

You can change liboqs' installation directory by configuring the build to use an
alternative path, e.g., `C:\liboqs`, by replacing the first CMake line above by

```shell
cmake -S liboqs -B liboqs/build -DCMAKE_INSTALL_PREFIX="C:\liboqs" -DBUILD_SHARED_LIBS=ON
```

### Configure and install the wrapper

Execute in a Terminal/Console/Administrator Command Prompt

```shell
git clone --depth=1 https://github.com/open-quantum-safe/liboqs-cpp
cmake -S liboqs-cpp -B liboqs-cpp/build
cmake --build liboqs-cpp/build --target install
```

### Build the examples

Execute, on UNIX-like platforms

```shell
cmake --build liboqs-cpp/build --target examples --parallel 8
```

and, on Windows platforms

```shell
cmake --build liboqs-cpp/build --target examples -DLIBOQS_INCLUDE_DIR="C:\Program Files (x86)\liboqs\include" -DLIBOQS_LIB_DIR="C:\Program Files (x86)\liboqs\lib" --parallel 8
```

Note that you may need to change the flags `-DLIBOQS_INCLUDE_DIR`
and `-DLIBOQS_LIB_DIR` to point to the correct location of `liboqs` in case you
installed it in a non-standard location.

To build only a specific target, e.g. `examples/kem`, specify the target as the
argument of the `cmake` command, e.g.,

```shell
cmake --build liboqs-cpp/build --target kem
```

### Run the examples

Execute

```shell
liboqs-cpp/build/kem
liboqs-cpp/build/sig
liboqs-cpp/build/rand
```

Note that on Windows platforms, the location and the names of the built examples
may be slightly different, e.g., `liboqs-cpp/build/Debug/kem.exe`.

### Build and run the unit tests

Execute

```shell
cmake --build liboqs-cpp/build/unit_tests --target unit_tests --parallel 8
```

followed by

```shell
ctest --test-dir liboqs-cpp/build
```

---

## Installing liboqs-cpp and using it in standalone applications

liboqs-cpp is a header-only wrapper. To use liboqs-cpp, you only need
to

```cpp
#include <liboqs-cpp/oqs_cpp.hpp>
```

in your application, and have liboqs library installed as described above.
See [examples/standalone](https://github.com/open-quantum-safe/liboqs-cpp/tree/main/examples/standalone)
for a standalone example.

To avoid namespace pollution, liboqs-cpp includes all of its code inside the
namespace `oqs`. All the liboqs C API is located in the namespace `oqs::C`,
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

Support for alternative RNGs is provided by the `include/rand/rand.hpp` header
file, which exports its functions in `namespace oqs::rand`. This header file
must be explicitly included in order to activate the support for alternative
RNGs.

The wrapper also defines a high resolution timing class, `oqs::Timer<>`.

The examples in
the [`examples`](https://github.com/open-quantum-safe/liboqs-cpp/tree/main/examples)
directory are self-explanatory stand-alone applications and provide more details
about the wrapper's API and its usage.

---

## Documentation

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

```shell
latexmk -pdf refman.tex
```

from the `doc/latex` directory or compile the file `doc/latex/refman.tex` with
your LaTeX compiler. This will create the `doc/latex/refman.pdf` documentation
file. Consult your favourite LaTeX manual for how to compile/build LaTeX files
under your specific operating system.

---

## Docker

A self-explanatory minimalistic Docker file is provided
in [`Dockerfile`](https://github.com/open-quantum-safe/liboqs-cpp/tree/main/Dockerfile).

Build the image by executing

```shell
docker build -t oqs-cpp .
```

Run, e.g., the key encapsulation example by executing

```shell
docker run -it oqs-cpp sh -c "liboqs-cpp/build/kem"
```

Or, run the unit tests with

```shell
docker run -it oqs-cpp sh -c "ctest --test-dir liboqs-cpp/build"
```

In case you want to use the Docker container as a development environment, mount
your current project in the Docker
container with

```shell
docker run --rm -it --workdir=/app -v ${PWD}:/app oqs-cpp /bin/bash
```

---

## Limitations and security

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

---

## License

liboqs-cpp is licensed under the MIT License;
see [LICENSE](https://github.com/open-quantum-safe/liboqs-cpp/blob/main/LICENSE)
for details.

---

## Team

The Open Quantum Safe project is led
by [Douglas Stebila](https://www.douglas.stebila.ca/research/)
and [Michele Mosca](https://faculty.iqc.uwaterloo.ca/mmosca/) at the University
of Waterloo.

liboqs-cpp was developed by [Vlad Gheorghiu](https://vsoftco.github.io) at
[softwareQ Inc.](https://www.softwareq.ca) and at the University of Waterloo.

---

## Support

Financial support for the development of Open Quantum Safe has been provided by
Amazon Web Services and the Canadian Centre for Cyber Security.

We'd like to make a special acknowledgement to the companies who have dedicated
programmer time to contribute source code to OQS, including Amazon Web Services,
evolutionQ, softwareQ, and Microsoft Research.

Research projects which developed specific components of OQS have been supported
by various research grants, including funding from the Natural Sciences and
Engineering Research Council of Canada (NSERC); see the source papers for
funding acknowledgments.
