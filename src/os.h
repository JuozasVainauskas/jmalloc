#ifndef JMALLOC_OS_H
#define JMALLOC_OS_H

#include <stddef.h>

void *os_grow_memory(size_t size);
void *os_shrink_memory(size_t size);
bool os_is_top_memory(const void *end);

#endif
