#!/bin/bash
cd build
cl.exe /std:c++17 /DLL /LD ../keybindings.cpp -o keybound.dll
cd ..
