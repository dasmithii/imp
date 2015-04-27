#ifndef BASELINE_HASH
#define BASELINE_HASH
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct HashNode {
	struct HashNode *next;
	void *key;
	void *value;
} HashNode;


typedef struct {
	HashNode **buckets;
	size_t bucketCount;
	size_t entryCount;
	size_t keySize;
	size_t valueSize;
	uint32_t (*hash)(const void*);
	int (*cmp)(const void*, const void*);
} Hash;


typedef uint32_t (*HashFunction)(const void*);
typedef int (*CompareFunction)(const void*, const void*);



// Management & Allocation.
int Hash_init(Hash*, size_t, size_t, HashFunction, CompareFunction);
void Hash_clear(Hash*);
void Hash_clean(Hash*);

// Storage & Removal.
int Hash_insert(Hash*, const void*, const void*);
bool Hash_contains(const Hash*, const void*);
void Hash_remove(Hash*, const void*);
void *Hash_hook(const Hash*, const void*);
void Hash_fetch(const Hash*, const void*, void*);

// Extras.
void Hash_forEach(Hash*, void (*)(const void*, void*));


#endif