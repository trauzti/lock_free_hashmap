CFLAGS= -ggdb3 -O3 -std=c++0x -levent -pthread -lpthread # -ljemalloc
#CFLAGS="-pg"

target: all

all: test_lock_free_hashmap 

#server: lock_free_hashtable.o socket_server.o readqueue.o
#	g++ $(CFLAGS) -o $@ $^ -levent -lpthread

# TOOD: make this work
#socket_server.o: socket_server.cc readqueue.c readqueue.h
#	g++ $(CFLAGS) -c socket_server.cc -levent -lpthread

readqueue.o: readqueue.c
	g++ $(CFLAGS) -c readqueue.c -levent -lpthread

hash.o: hash.cpp
	g++ $(CFLAGS) -c hash.cpp 

gc.o: gc.cc gc.h
	g++ $(CFLAGS) -c gc.cc

lock_free_hashtable.o: lock_free_hashtable.cc 
	g++ $(CFLAGS) -c $^

test_lock_free_hashmap: lock_free_hashtable.o hash.o gc.o
	g++ $(CFLAGS)  test_lock_free_hashmap.cpp $^ -o $@

.PHONY: clean
clean:
	-rm -f *.o server core test_lock_free_hashmap
	

