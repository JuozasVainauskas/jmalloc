#ifndef JMALLOC_HEADER_H
#define JMALLOC_HEADER_H

#include <stddef.h>

// Bytes alignment number for target platform. 16 on x86_64
// It means that JMalloc's address pointers must be divisible by JMALLOC_ALIGNMENT
#define JMALLOC_ALIGNMENT alignof(max_align_t)

// Align requested memory up. e.g. 5 -> 16, 17 -> 32
#define JMALLOC_ALIGN_UP(x) (((x) + (JMALLOC_ALIGNMENT - 1)) & ~(JMALLOC_ALIGNMENT - 1))

typedef struct header {
    // Raise the whole structs alignment to JMALLOC_ALIGNMENT
    // This way sizeof(header_t) will round up to a multiple of the alignment (e.g. 24 -> 32, 40 -> 48)
    alignas(JMALLOC_ALIGNMENT) size_t size;
    unsigned is_free;
    struct header *next;
} header_t;

header_t *get_free_block(size_t size);

static_assert((JMALLOC_ALIGNMENT & (JMALLOC_ALIGNMENT - 1)) == 0,
               "alignment must be a power of two");
static_assert(alignof(header_t) == JMALLOC_ALIGNMENT,
               "header must be aligned to JMALLOC_ALIGNMENT");
static_assert(sizeof(header_t) % JMALLOC_ALIGNMENT == 0,
               "header size must be a multiple of JMALLOC_ALIGNMENT");

static_assert(JMALLOC_ALIGN_UP(0)                     == 0);
static_assert(JMALLOC_ALIGN_UP(1)                     == JMALLOC_ALIGNMENT);
static_assert(JMALLOC_ALIGN_UP(JMALLOC_ALIGNMENT - 1) == JMALLOC_ALIGNMENT);
static_assert(JMALLOC_ALIGN_UP(JMALLOC_ALIGNMENT)     == JMALLOC_ALIGNMENT);
static_assert(JMALLOC_ALIGN_UP(JMALLOC_ALIGNMENT + 1) == 2 * JMALLOC_ALIGNMENT);
#endif
