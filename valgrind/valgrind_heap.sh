#!/bin/bash

valgrind --tool=massif -v xmoto 2> valgrind.log
