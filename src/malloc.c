#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "heap.h"
#include "os.h"
#include <stdint.h>

static heap_t heap;
static pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
    if (!size) {
        return NULL;
    }

    size_t overflow_limit = SIZE_MAX - sizeof(header_t) - (JMALLOC_ALIGNMENT - 1);
    if (size > overflow_limit) {
        return NULL;
    }
    size = JMALLOC_ALIGN_UP(size);

    pthread_mutex_lock(&global_malloc_lock);
    header_t *header = heap_find_free_block(&heap, size);
    if (header) {
        header->is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        // Return memory address after the header
        return (void*)(header + 1);
    }

    size_t total_size = sizeof(header_t) + size;
    void *block = os_grow_memory(total_size);
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = heap_add_block(&heap, block, size);
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}

void free(void *block) {
    if (!block) {
        return;
    }

    pthread_mutex_lock(&global_malloc_lock);
    header_t *header = (header_t*)block - 1;
    void *program_break = sbrk(0);
    void *block_end = (char*)(block) + header->size;

    // If block is not at the end of the memory, mark it as free and reuse
    if (block_end != program_break) {
        header->is_free = 1;
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    heap_remove_tail(&heap);
    sbrk(0 - sizeof(header_t) - header->size);
    pthread_mutex_unlock(&global_malloc_lock);
    return;
}

void *calloc(size_t num, size_t nsize) {
    if (!num || !nsize) {
        return NULL;
    }

    size_t size = num * nsize;
    // Check for multiplicative overflow
    if (nsize != size / num) {
        return NULL;
    }

    void *block = malloc(size);
    if (!block) {
        return NULL;
    }

    memset(block, 0, size);
    return block;
}

void *realloc(void *block, size_t size) {
    if (!size) {
        free(block);
        return NULL;
    } else if (!block) {
        return malloc(size);
    }

    header_t *header = (header_t*)block - 1;
    if (header->size >= size) {
        return block;
    }

    void* ret = malloc(size);
    if (!ret) {
        return NULL;
    }
    memcpy(ret, block, header->size);
    free(block);
    return ret;
}
