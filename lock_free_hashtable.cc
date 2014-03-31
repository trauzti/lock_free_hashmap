#include <cassert>
#include <cstdio>
#include <cstring>
#include "lock_free_hashtable.h"
#include "hash.hpp"

#define seed 1337

inline static uint32_t int_hash(uint32_t h)
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}


inline static uint32_t str_hash(const char *s) {
  int nkey = strlen(s);
  uint32_t h;
  MurmurHash3_x64_32 (s, nkey, seed, &h);
  return h;
}


lock_free_hashtable::lock_free_hashtable(uint32_t size)
{
  // Initialize cells
  assert((size & (size - 1)) == 0);   // Must be a power of 2
  table_size = size;
  printf("sizeof(Item *)=%u\n", sizeof(Item *));
  printf("Allocating %uMB\n", (sizeof(Item *) * size) >> 20);
  items = new Item *[size];
  for (int i = 0; i < size; i++)
    items[i] = NULL;
  clear();
}


void lock_free_hashtable::set_item(const char *key, Item *nit)
{
  // key lock
  assert(key != NULL);
  assert(nit->value != NULL);
  int nkey = nit->nkey;

  for (uint32_t idx = str_hash(key); idx < table_size; ++idx)
  {
    idx &= table_size - 1;
    Item *it = items[idx];

    if (it == NULL) {
      bool exchanged = __sync_bool_compare_and_swap (&items[idx], it, nit);
      if (!exchanged)
        continue;       // Another thread just stole it from underneath us.
      else {
        // success
      }
    }
    else {
      if ((nkey == it->nkey) && (memcmp(key, it->key, nkey) == 0))
      {
        // already stored !! update the value?
        //printf("already stored\n");
        // atomically copy the value?
        // need to delete the new or the old item

        // replace it with nit
        // add it to garbage instead of free
        bool exchanged = __sync_bool_compare_and_swap (&items[idx], it, nit);
        if (!exchanged)	{
          assert(0);
          // what do we do here ?
          // Another thread just stole it from underneath us.
        }
      }
      else
      {
        continue;           // Usually, it contains another key. Keep probing.
      }
    }
    return;
  }
}

// the user must free this
item *lock_free_hashtable::get_item(const char *key)
{
  assert(key != NULL);
  int nkey = strlen(key);

  for (uint32_t idx = str_hash(key); idx < table_size; idx++)
  {
    idx &= table_size - 1;

    Item *it = items[idx];
    if (it == NULL)
      return NULL;
    if (nkey == it->nkey)
      if (memcmp(key, it->key, nkey) == 0)
        return it;
  }

  return NULL;
}

void lock_free_hashtable::delete_item(const char *key)
{
  assert(key != NULL);

  uint32_t i=str_hash(key);
  int nkey = strlen(key);
  i &= table_size - 1;
  item *it = items[i];
  if (!it)
    return;	// Key is not in the table.

  uint32_t j=i+1; // the next item, which we might want to move around

  while (1) {
    // mark slot as unoccupied
    if (!__sync_bool_compare_and_swap (&items[i], it, NULL))
      return; // Abort if another thread deleted it before us.
    for (;;++j) {
      j &= table_size - 1;
      it=items[j];
      if (it == NULL)
        return; // No more occupied slots to move.
      assert(it->key != NULL);
      uint32_t k=str_hash(it->key)&(table_size-1); // Re-hash the key stored in slot j
      if ( (i<=j) ? ((i<k)&&(k<=j)) : ((i<k)||(k<=j)) ) // If k is between i and j (cyclically)
        continue;
      break;
    }
    // k is before i, move item j to slot i
    if (!__sync_bool_compare_and_swap (&items[i], NULL, it))
      return; // Abort if another thread just wrote in the deleted cell.
    //printf("moved to an earlier empty slot\n");
    i=j; // Do the same procedure for the emptied slot.
    it = items[i];
  }
}

void lock_free_hashtable::resize() {
  // TODO: if the occupancy reaches 95% do resize
}

void lock_free_hashtable::terminate()
{
  Item *it;
  for (int i = 0; i < table_size; i++) {
    it = items[i];
    if (it != NULL) {
      delete it;
    }
  }
  delete [] items;
}

void lock_free_hashtable::clear()
{
  memset(items, 0, sizeof(Item *) * table_size);
}

void lock_free_hashtable::print_table()
{
  for (int i=0;i<table_size;i++)
  {
    if (items[i] == NULL)
    {
      printf("__|");
    }
    else
    {
      printf("%s|", items[i]->key);
    }
  }
  printf("\n");
}
