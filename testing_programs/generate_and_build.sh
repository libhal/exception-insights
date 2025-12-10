#!/bin/sh
mkdir build/
g++ -o build/demo_class -fdump-ipa-whole-program -flto -O0 demo_class.cpp demo_two.cpp
g++ -static simple.cpp -o build/simple
cd build/
mv demo_class.wpa.*.whole-program multi_tu.whole-program
echo Built example program with multiple TUs.