#!/bin/sh
lipo -create _esy/ios.simulator.x86_64/build/default.ios.simulator.x86_64/bin/hello.exe.o _esy/ios.arm64/build/default.ios.arm64/bin/hello.exe.o -output hello.exe.o
