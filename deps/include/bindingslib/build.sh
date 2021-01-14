#!/bin/bash
cd build
# I hate. Hate. Hate everything Windows command line. The / flags are horrible standard, hard to type,
# and it basically makes reasonable folder navigation impossible, as / is also easier to type than \
# This simple fact alone, could make one hate the *entire* Windows eco system. Even if it is trivial as hell.

# c++17 required due to use of structured bindings. If you don't build c++17, in new projects, then what are we really doing here?
cl.exe /std:c++17 /DLL /LD ../keybindings.cpp /out:keybound.dll
cd ..
