#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>

#define NUM_REQUESTS (1 << 22)

#include "lock_free_hashtable.h"
#define item Item

std::atomic<uint32_t> stored(0);
int NUM_THREADS;
lock_free_hashtable* hashtable;

void many_requests(int id) {
    struct drand48_data randBuffer;
    long int x;
    uint32_t key;
    bool deletenow;
    char skey[128];
    srand48_r(id + 1, &randBuffer);
    printf("Thread %d starting to perform %d requests!\n", id, NUM_REQUESTS);
    for (int i = 0; i < NUM_REQUESTS; i++) {
        lrand48_r(&randBuffer, &x);
        key = x % ((1<< 22) -101 );
        ++key; // prevent 0
        memset(skey, 0, 128*sizeof(char));
        sprintf(skey, "%d", key);
        //printf("Thread %d requesting key %d\n", id, key);

        // TODO: take keylock
        item *it = hashtable->get_item(skey);
        if (it == NULL) {
            item *nit = new item(skey, skey);
            hashtable->set_item(skey, nit);
            stored += 1;
        } else {
            lrand48_r(&randBuffer, &x);
            int q = x % 101;
            assert(strcmp(it->value, skey) == 0);
            if (q > 50) {
                hashtable->delete_item(skey);
                stored -= 1;
                delete it;
            }
        }
        // TODO: unlock the keylock
    }
    printf("Thread %d done!\n", id);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <# threads>\n", argv[0]);
        exit(0);
    }
    NUM_THREADS = atoi(argv[1]);
    uint32_t size = (1 << 22);
    printf("Initializing hashtable of size %u\n", size);
    hashtable = new lock_free_hashtable(size);

    /*
       item *it;
       const char test[] = "1000";
       it = hashtable->get_item(test);
       assert(!it);
       item *nit = new item(test, test);
       hashtable->set_item(test, nit);
       it = hashtable->get_item(test);
       assert(strcmp(it->value, test) == 0);
       hashtable->delete_item(test);
       */

    printf("Spawning %d threads\n", NUM_THREADS);

    thread *t = new thread[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        t[i] = thread(many_requests, i);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        t[i].join();
    }
    printf("Occupancy: %.2f%% (%u/%u)\n", (100 * stored.load() / (0.0 + size)), stored.load(), size);
    return 0;
}

