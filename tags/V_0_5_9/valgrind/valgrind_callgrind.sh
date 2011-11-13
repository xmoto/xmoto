#!/bin/bash

valgrind  --tool=callgrind ../src/xmoto ../bin/Levels/test/test1000smoke.lvl

#kcachegrind callgrind.out.17833 &
