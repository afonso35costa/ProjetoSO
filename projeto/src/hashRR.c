#include "hashRR.h"
#include "queue.h"
#include <glib.h>

GHashTable* hash_create() {
    return g_hash_table_new(g_direct_hash, g_direct_equal);
}

void add_to_hashRR(GHashTable *hashRR, int key, Queue *value) {
    g_hash_table_insert(hashRR, GINT_TO_POINTER(key), value);
}

Queue* get_from_hashRR(GHashTable *hashRR, int key) {
    return g_hash_table_lookup(hashRR, GINT_TO_POINTER(key));
}

void remove_from_hashRR(GHashTable *hashRR, int key) {
    g_hash_table_remove(hashRR, GINT_TO_POINTER(key));
}

int cmd_count_in_all_queues(GHashTable *hashRR) {
    int total = 0;
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, hashRR);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        Queue *fila = value;
        total += fila->size;
    }
    return total;
}
