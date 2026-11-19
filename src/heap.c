#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "heap.h"
#include <stdint.h>

header_t *heap_find_free_block(heap_t *heap, size_t size) {
    header_t *curr = heap->head;
    while(curr) {
        if (curr->is_free && curr->size >= size) {
            return curr;
        }

        curr = curr->next;
    }
    return NULL;
}

header_t *heap_add_block(heap_t *heap, void *block, size_t size) {
    header_t *header = block;
    header->size = size;
    header->is_free = 0;
    header->next = NULL;
    if (!heap->head) {
        heap->head = header;
    }
    if (heap->tail) {
        heap->tail->next = header;
    }
    heap->tail = header;
    return header;
}

void heap_remove_tail(heap_t *heap) {
    if (heap->head == heap->tail) {
        heap->head = heap->tail = NULL;
        return;
    }
    header_t *curr = heap->head;
    while (curr->next != heap->tail) {
        curr = curr->next;
    }
    curr->next = NULL;
    heap->tail = curr;
}
