#ifndef HASH_RR_H
#define HASH_RR_H
#include <glib.h>
#include "queue.h"

GHashTable* hash_create();
void add_to_hashRR(GHashTable *hashRR, int key, Queue *value);
Queue* get_from_hashRR(GHashTable *hashRR, int key);
void remove_from_hashRR(GHashTable *hashRR, int key);

#endif