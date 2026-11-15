#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "header.h"
#include <stdint.h>

header_t *head = NULL, *tail = NULL;
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
    header_t *header = get_free_block(size);
    if (header) {
        header->is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        // Return memory address after the header
        return (void*)(header + 1);
    }

    // Make sure the new block starts on an aligned address
    uintptr_t start = (uintptr_t)sbrk(0);
    size_t pad = JMALLOC_ALIGN_UP(start) - start;
    if (pad && sbrk(pad) == (void *)-1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    size_t total_size = sizeof(header_t) + size;
    void *block = sbrk(total_size);
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = block;
    header->size = size;
    header->is_free = 0;
    header->next = NULL;
    if (!head) {
        head = header;
    }
    if (tail) {
        tail->next = header;
    }
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}

header_t *get_free_block(size_t size) {
    header_t *curr = head;
    while(curr) {
        if (curr->is_free && curr->size >= size) {
            return curr;
        }

        curr = curr->next;
    }
    return NULL;
}

void free(void *block) {
    if (!block) {
        return;
    }

    pthread_mutex_lock(&global_malloc_lock);
    header_t *header = (header_t*)block - 1;
    void *program_break = sbrk(0);
    void *block_end = (char*)(block) + header->size;
    if (block_end != program_break) {
        header->is_free = 1;
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    if (head == tail) {
        head = tail = NULL;
    } else {
        header_t *tmp = head;
        while (tmp) {
            if (tmp->next == tail) {
                tmp->next = NULL;
                tail = tmp;
            }
            tmp = tmp->next;
        }
    }
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
