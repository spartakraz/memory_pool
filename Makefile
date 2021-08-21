build_lib: compile_lib
		ar rcs bin/libmempool.a bin/mempool.o
compile_lib: mempool.c
		gcc -c -g -std=gnu99 -D_GNU_SOURCE=1 -DTEST_MODE=1 -Wall mempool.c -o bin/mempool.o -lpthread
clean:
		rm -rvf bin/libmempool.a bin/mempool.o
