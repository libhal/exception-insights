#!/bin/sh
echo -e hello!
mkdir build/
g++ -o build/demo_class -fdump-ipa-whole-program -flto -O0 demo_class.cpp demo_two.cpp