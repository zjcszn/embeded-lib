#!/bin/sh

mkdir build

find src/ -name "*.java" > listFile.tmp

javac -target 1.6 -source 1.6 -d build @listFile.tmp

jar cfm gendyncode.jar manifest-dyncCode.mf -C build/ com/

rm listFile.tmp
rm -r build
