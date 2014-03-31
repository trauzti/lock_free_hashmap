#include <atomic>

#include "item.hpp"
#include "hash.hpp"

#define Item item

class lock_free_hashtable
{

	private:    
		Item **items;
		uint32_t table_size;

	public:
		lock_free_hashtable(uint32_t size);

		~lock_free_hashtable() {
      terminate();
    };
		
    void terminate();

		void set_item(const char *key, Item *nit);
		Item *get_item(const char *key);
		void delete_item(const char *key);
		void clear();
		void print_table();
    void resize();
};
