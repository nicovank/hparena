#ifndef HUGEPAGE_ARENA_H
#define HUGEPAGE_ARENA_H

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#define K_HUGEPAGE_SIZE (1UL << 21)

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define K_ASAN_ENABLED 1
#else
#define K_ASAN_ENABLED 0
#endif
#else
#define K_ASAN_ENABLED 0
#endif

#if K_ASAN_ENABLED
#include <sanitizer/asan_interface.h>
#define K_ASAN_REDZONE_EXTRA_BYTES 8
#else
#define K_ASAN_REDZONE_EXTRA_BYTES 0
#define __asan_poison_memory_region(A, B)                                      \
    do {                                                                       \
        (void) (A);                                                            \
        (void) (B);                                                            \
    } while (0)
#define __asan_unpoison_memory_region(A, B)                                    \
    do {                                                                       \
        (void) (A);                                                            \
        (void) (B);                                                            \
    } while (0)
#endif

size_t alignUp(size_t size, size_t alignment) {
    assert(alignment != 0);                     // Is not 0.
    assert((alignment & (alignment - 1)) == 0); // Is a power of 2.
    return (size + alignment - 1) & ~(alignment - 1);
}

typedef struct ArenaChunk {
    void* ptr;
    size_t size;
    struct ArenaChunk* next;
} ArenaChunk;

typedef struct HugepageArena {
    ArenaChunk* head;
    uintptr_t currentBumpPtr;
    uintptr_t currentChunkEnd;
} HugepageArena;

HugepageArena* arena_init(void) {
    HugepageArena* arena = (HugepageArena*) malloc(sizeof(HugepageArena));
    if (arena == NULL) {
        fprintf(stderr, "[ERROR] malloc failed in arena_init...\n");
        return NULL;
    }

    ArenaChunk* chunk = (ArenaChunk*) malloc(sizeof(ArenaChunk));
    if (chunk == NULL) {
        fprintf(stderr, "[ERROR] malloc failed in arena_init...\n");
        free(arena);
        return NULL;
    }

    chunk->ptr = NULL;
    chunk->size = 0;
    chunk->next = NULL;
    arena->head = chunk;
    arena->currentBumpPtr = 0;
    arena->currentChunkEnd = 0;

    return arena;
}

void arena_free(HugepageArena* arena) {
    ArenaChunk* chunk = arena->head;
    while (chunk != NULL) {
        ArenaChunk* next = chunk->next;
        munmap(chunk->ptr, chunk->size);
        free(chunk);
        chunk = next;
    }
    free(arena);
}

void* arena_alloc(HugepageArena* arena, size_t size, size_t alignment) {
    if (alignment > 4096) {
        return NULL;
    }

    // fprintf(stderr, "[INFO] arena_alloc(%zu, %zu)\n", size, alignment);

    uintptr_t alignedPtr = alignUp(arena->currentBumpPtr, alignment);
    if (alignedPtr + size + K_ASAN_REDZONE_EXTRA_BYTES
        <= arena->currentChunkEnd) {
        arena->currentBumpPtr = alignedPtr + size + K_ASAN_REDZONE_EXTRA_BYTES;
        __asan_unpoison_memory_region((void*) alignedPtr, size);
        return (void*) alignedPtr;
    }

    // TODO: Make this metadata part of the allocation?
    ArenaChunk* newChunk = (ArenaChunk*) malloc(sizeof(ArenaChunk));
    if (newChunk == NULL) {
        fprintf(stderr, "[ERROR] malloc failed in arena_alloc...\n");
        return NULL;
    }

    size_t requestSize = size <= K_HUGEPAGE_SIZE
                             ? K_HUGEPAGE_SIZE
                             : alignUp(size, K_HUGEPAGE_SIZE);

    newChunk->ptr = mmap(NULL, requestSize, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (newChunk->ptr == MAP_FAILED) {
        fprintf(stderr, "[ERROR] mmap failed in arena_alloc...\n");
        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        free(newChunk);
        return NULL;
    }

    // Presumably, this is at least 4K-aligned, so no need to re-align it.
    assert(alignUp((uintptr_t) newChunk->ptr, 4096)
           == (uintptr_t) newChunk->ptr);

    newChunk->size = requestSize;
    __asan_poison_memory_region(newChunk->ptr, requestSize);
    __asan_unpoison_memory_region(newChunk->ptr, size);

    newChunk->next = arena->head;
    arena->head = newChunk;
    arena->currentBumpPtr
        = ((uintptr_t) newChunk->ptr) + size + K_ASAN_REDZONE_EXTRA_BYTES;
    arena->currentChunkEnd = ((uintptr_t) newChunk->ptr) + newChunk->size;

    return newChunk->ptr;
}

#endif // HUGEPAGE_ARENA_H
