#include <stdint.h>
#include <unistd.h>
#include "heap.h"
#include "os.h"

void *os_grow_memory(size_t size) {
    // Make sure the new block starts on an aligned address
    uintptr_t start = (uintptr_t)sbrk(0);
    size_t pad = JMALLOC_ALIGN_UP(start) - start;
    if (pad && sbrk(pad) == (void*) - 1) {
        return NULL;
    }

    void *mem = sbrk(size);
    return mem == (void *)-1 ? NULL : mem;
}

void *os_shrink_memory(size_t size) {
    sbrk(-(intptr_t)size);

}
bool os_is_top_memory(const void *end) {
    return end == sbrk(0);
}
