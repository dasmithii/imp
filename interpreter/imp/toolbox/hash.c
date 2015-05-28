// Toolkit - a collection of utilities for C-based software projects.
// Douglas Adam Smith II
// github.com/dasmithii/toolkit


#include <string.h>
#include "hash.h"
#include "debug.h"
#define DEFAULT_BUCKET_COUNT 100


static size_t bucketIndex(const Hash *self, const void *key)
{
	assume_ptr(self);
	uint32_t hash = self->hash(key);
	return hash % self->bucketCount;
}


static HashNode *hookBucket(const Hash *self, const void *key)
{
	assume_ptr(self);
	size_t index = bucketIndex(self, key);
	return self->buckets[index];
}


static HashNode *hookNode(const Hash *self, const void *key)
{
	HashNode *node = hookBucket(self, key);
	while(node){
		if(self->cmp(key, node->key) == 0)
			return node;
		node = node->next;
	}
	return NULL;
}


static void freeNode(HashNode *self)
{
	assume_ptr(self);
	assume_ptrs(self->key, self->value);
	free(self->key);
	free(self->value);
	free(self);
}


static void freeBucket(HashNode *self)
{
	while(self){
		HashNode *temp = self->next;
		freeNode(self);
		self = temp;
	}
}


static void squeezeNext(HashNode *self)
{
	assume_ptr(self);
	HashNode *next = self->next;
	if(next){
		self->next = next->next;
		freeNode(next);
	}
}


static void clearBucket(Hash *self, size_t i)
{
	HashNode *node = self->buckets[i];
	while(node){
		HashNode *temp = node->next;
		freeNode(node);
		self->entryCount--;
		node = temp;
	}
	self->buckets[i] = NULL;
}


static HashNode *newNode(Hash *hash, const void *key, const void *value)
{
	HashNode *ret = malloc(sizeof(HashNode));
	if(!ret)
		return NULL;
	ret->next = NULL;
	ret->key = malloc(hash->keySize);
	ret->value = malloc(hash->valueSize);
	if(!ret->key || !ret->value){
		free(ret);
		return NULL;
	}
	memmove(ret->key, key, hash->keySize);
	memmove(ret->value, value, hash->valueSize);
	return ret;
}


static int bucketInsert(Hash *self, size_t i, const void *key, const void *value)
{
	HashNode *node = self->buckets[i];
	while(node->next){
		if(self->cmp(key, node->key) == 0){
			memmove(node->value, value, self->valueSize);
			return 0;
		}
		node = node->next;
	}
	node->next = newNode(self, key, value);
	return node->next? 0:1;
}


int Hash_init(Hash *self
	        , size_t keySize
	        , size_t valueSize
	        , HashFunction hash
	        , CompareFunction cmp)
{
	assume_ptr(self);
	self->buckets = calloc(DEFAULT_BUCKET_COUNT, sizeof(HashNode*));
	if(!self->buckets)
		return 1;
	self->bucketCount = DEFAULT_BUCKET_COUNT;
	self->entryCount  = 0;
	self->keySize = keySize;
	self->valueSize = valueSize;
	self->hash = hash;
	self->cmp = cmp;
	return 0;
}

void Hash_clear(Hash *self)
{
	assume_ptr(self);
	for(int i = 0; i < self->bucketCount; ++i)
		clearBucket(self, i);
}


void Hash_clean(Hash *self)
{
	assume_ptr(self);
	Hash_clear(self);
	free(self->buckets);
	self->buckets = NULL;
	self->bucketCount = 0;
}


int Hash_insert(Hash *self, const void *key, const void *value)
{
	size_t i = bucketIndex(self, key);
	if(!self->buckets[i]){
		self->buckets[i] = newNode(self, key, value);
		return self->buckets[i]? 0:1;
	}
	return bucketInsert(self, i, key, value);
}


bool Hash_contains(const Hash *self, const void *key)
{
	return hookNode(self, key)? true:false;
}


void Hash_remove(Hash *self, const void *key)
{
	size_t i = bucketIndex(self, key);
	HashNode *node = self->buckets[i];
	if(node){
		if(self->cmp(key, node->key) == 0){
			clearBucket(self, i);
		} else {
			while(node->next){
				if(self->cmp(key, node->next->key) == 0){
					squeezeNext(node);
					self->entryCount--;
					return;
				}
			}
		}
	}
}


void *Hash_hook(const Hash *self, const void *key)
{
	HashNode *node = hookNode(self, key);
	return node? node->value:NULL;
}


void Hash_fetch(const Hash *self, const void *key, void *dest)
{
	assume_ptrs(self, dest);
	void *from = Hash_hook(self, key);
	memmove(dest, from, self->valueSize);
}


void Hash_forEach(Hash *self, void (*func)(const void*, void*))
{
	for(int i = 0; i < self->bucketCount; ++i){
		HashNode *node = self->buckets[i];
		while(node){
			func(node->key, node->value);
			node = node->next;
		}
	}
}
