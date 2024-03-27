# Version 0.10.0 - March 27, 2024

- Replaced CHANGES by
  [CHANGES.md](https://github.com/open-quantum-safe/liboqs-cpp/blob/main/CHANGES.md),
  as we now use Markdown format to keep track of changes in new releases
- Removed GoogleTest dependency; if not detected, it is installed automatically
  as build dependency by CMake
- Bumped GoogleTest version to HEAD latest, as
  [recommended by Google](https://github.com/google/googletest?tab=readme-ov-file#live-at-head)
- Removed the NIST PRNG as the latter is no longer exposed by liboqs' public
  API

# Version 0.9.1 - November 1, 2023

- Added support for install/uninstall via CMake, and a standalone example in
  ["examples/standalone"].

# Version 0.9.0 - October 30, 2023

- No modifications, release bumped to match the latest release of liboqs

# Version 0.8.0 - July 5, 2023

- This is a maintenance release, minor fixes
- Minimalistic Docker support
- Removed AppVeyor and CircleCI, all continuous integration is now done via
  GitHub actions

# Version 0.7.2 - September 1, 2022

- Added library version retrieval functions
  - `std::string oqs::oqs_version()`
  - `std::string oqs::oqs_cpp_version()`
- Bumped GoogleTest version to 1.12.1
  [commit](https://github.com/google/googletest/commit/58d77fa8070e8cec2dc1ed015d66b454c8d78850)

# Version 0.7.1 - January 5, 2022

- Release numbering updated to match liboqs
- Integrated the unit tests with the main project, so now the unit tests are
  automatically compiled along with the examples, and can now be run by typing
  `make test` or `ctest`. Use `GTEST_COLOR=1 ARGS="-V" make test` or
  `GTEST_COLOR=1 ctest -V` for coloured verbose testing output.
- CMake minimum required version bumped to 3.10 (3.12 for macOS) for automatic
  unit tests detection by CMake
- Switched continuous integration from Travis CI to CircleCI, we now support
  macOS & Linux (CircleCI) and Windows (AppVeyor)

# Version 0.4.0 - November 28, 2020

- Renamed `master` branch to `main`

# Version 0.3.0 - June 10, 2020

- Removed the Visual Studio solution (since it can be automatically generated
  by CMake), as we prefer to use CMake uniformly across all platforms
- Minor fixes

# Version 0.2.2 - January 16, 2020

- Added additional RNG example project to Visual Studio solution

# Version 0.2.1 - November 2, 2019

- Added support for RNGs from `<oqs/rand.h>`
- Concurrent unit testing

# Version 0.2.0 - October 8, 2019

- Minor changes to accomodate for liboqs API changes

# Version 0.1.2 - July 9, 2019

- Added MSVC support for CMake
- Updated Google Test to version 1.8.1
- Bugfix in `oqs::Signature::sign()`

# Version 0.1.1 - May 29, 2019

- Minor API change in `oqs_cpp.h`: `Signature::alg_details_::length_signature`
  is replaced by `Signature::alg_details_::max_length_signature`

# Version 0.1.0 - April 23, 2019

- Initial release
