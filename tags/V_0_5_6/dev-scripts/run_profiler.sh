#valgrind --tool=callgrind ./xmoto -nowww -win -ugly -res 800x480 -level 5
export VALGRIND_OPTS="--trace-jump=yes --dump-instr=yes"
valgrind --tool=callgrind ./xmoto $XMOTO_OPTS $@
