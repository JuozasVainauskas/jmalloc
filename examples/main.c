#include <stdio.h>
#include <stdlib.h>
#include "../src/header.h"

int main(void) {
    void* ptr = malloc(sizeof(5 * sizeof(int)));
    printf("malloc(sizeof(5 * sizeof(int))): %p\n", ptr);

    void *ptr_two = malloc(5);
    printf("malloc(5): %p\n", ptr_two);

    return 0;
}
