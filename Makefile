CFLAGS= -ggdb3 -O3 -std=c++11 -levent -lpthread # -ljemalloc
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

lock_free_hashtable.o: lock_free_hashtable.cc 
	g++ $(CFLAGS) -c $^

test_lock_free_hashmap: lock_free_hashtable.o hash.o
	g++ $(CFLAGS)  test_lock_free_hashmap.cpp lock_free_hashtable.o hash.o -o $@

.PHONY: clean
clean:
	-rm -f *.o server core test_lock_free_hashmap
	

