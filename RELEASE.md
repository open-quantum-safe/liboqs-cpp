liboqs-cpp version 0.1.2
========================

About
-----

The **Open Quantum Safe (OQS) project** has the goal of developing and prototyping quantum-resistant cryptography.  More information on OQS can be found on our website: https://openquantumsafe.org/ and on Github at https://github.com/open-quantum-safe/.  

**liboqs** is an open source C library for quantum-resistant cryptographic algorithms.  See more about liboqs at [https://github.com/open-quantum-safe/liboqs/](https://github.com/open-quantum-safe/liboqs/), including a list of supported algorithms.

**liboqs-cpp** is an open source C++ wrapper for the liboqs C library for quantum-resistant cryptographic algorithms.  Details about liboqs-cpp can be found in [README.md](https://github.com/open-quantum-safe/liboqs-cpp/blob/master/README.md).  See in particular limitations on intended use.

Release notes
=============

This release of liboqs-cpp was released on July 12, 2019. Its release page on GitHub is https://github.com/open-quantum-safe/liboqs-cpp/releases/tag/0.1.2.

What's New
----------

For a list of changes see [CHANGES.txt](https://github.com/open-quantum-safe/liboqs-cpp/blob/master/CHANGES.txt).

This solution implements a header-only C++ wrapper in C++11 for the C OQS library. It contains the main header file for the wrapper, as well as examples, documentation, and unit tests.

liboqs-cpp can be compiled against liboqs master branch, and makes available all digital signature schemes and key encapsulation mechanisms from liboqs.
