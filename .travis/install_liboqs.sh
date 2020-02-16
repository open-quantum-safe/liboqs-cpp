#!/bin/sh

git clone https://github.com/open-quantum-safe/liboqs
cd liboqs
git checkout master
mkdir build && cd build
cmake -GNinja ..
ninja
ninja install