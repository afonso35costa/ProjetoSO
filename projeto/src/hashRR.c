#include "hashRR.h"
#include "queue.h"
#include <glib.h>

GHashTable* hash_create() {
    return g_hash_table_new(g_direct_hash, g_direct_equal);
}

/*
 * add_to_hashRR
 * -------------
 * Associa uma Queue a um userId na tabela hashRR.
 */
void add_to_hashRR(GHashTable *hashRR, int key, Queue *value) {
    g_hash_table_insert(hashRR, GINT_TO_POINTER(key), value);
}

/*
 * get_from_hashRR
 * ----------------
 * Retorna a Queue associada a um userId, ou NULL se não existir.
 */
Queue* get_from_hashRR(GHashTable *hashRR, int key) {
    return g_hash_table_lookup(hashRR, GINT_TO_POINTER(key));
}

/*
 * remove_from_hashRR
 * ------------------
 * Remove a associação de um userId da tabela.
 */
void remove_from_hashRR(GHashTable *hashRR, int key) {
    g_hash_table_remove(hashRR, GINT_TO_POINTER(key));
}

/*
 * cmd_count_in_all_queues
 * -----------------------
 * Conta o total de comandos presentes em todas as filas guardadas na hash.
 */
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
