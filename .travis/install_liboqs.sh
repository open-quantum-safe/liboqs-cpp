#!bin/sh

git clone https://github.com/open-quantum-safe/liboqs
cd liboqs
git checkout master
autoreconf -i
./configure
make clean
make -j
sudo make install
