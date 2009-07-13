#!/bin/bash

valgrind --tool=memcheck --leak-check=full xmoto 2> valgrind.log
