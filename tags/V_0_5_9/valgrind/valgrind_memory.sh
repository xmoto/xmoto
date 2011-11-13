#!/bin/bash

# --show-possibly-lost=no
valgrind --tool=memcheck --leak-check=full --show-reachable=yes xmoto 2> valgrind.log
