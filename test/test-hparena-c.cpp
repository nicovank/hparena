#include <vector>

#include <gtest/gtest.h>

#include <hparena/hparena.h>

TEST(AlignUpTest, BasicCases) {
    EXPECT_EQ(alignUp(1, 8), 8u);
    EXPECT_EQ(alignUp(8, 8), 8u);
    EXPECT_EQ(alignUp(9, 8), 16u);
    EXPECT_EQ(alignUp(15, 16), 16u);
    EXPECT_EQ(alignUp(17, 16), 32u);
}

// arena_alloc basic allocation
TEST(ArenaAllocTest, BasicAllocations) {
    HugepageArena* arena = arena_init();
    void* ptr1 = arena_alloc(arena, 64, 8);
    ASSERT_NE(ptr1, nullptr);
    void* ptr2 = arena_alloc(arena, 128, 16);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_NE(ptr1, ptr2);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr1) % 8, 0u);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 16, 0u);
    arena_free(arena);
}

TEST(ArenaAllocTest, MatchLZAllocs) {
    {
        HugepageArena* arena = arena_init();
        std::vector<std::size_t> sizes
            = {{88, 1400, 320, 112, 216, 4272, 184, 28352, 67108864}};
        for (const auto size : sizes) {
            char* pointer = (char*) arena_alloc(arena, size, 8);
            memset(pointer, 0, size);
        }
        arena_free(arena);
    }

    {
        HugepageArena* arena = arena_init();
        std::vector<std::size_t> sizes = {
            {88,  1400, 320, 112, 216, 4272, 184, 28352, 4194304, 112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112, 112, 112, 112,  112, 112,   112,     112, 112, 112,
             112, 112,  112}};
        for (const auto size : sizes) {
            char* pointer = (char*) arena_alloc(arena, size, 8);
            memset(pointer, 0, size);
        }
        arena_free(arena);
    }
}

// arena_alloc invalid alignment
TEST(ArenaAllocTest, InvalidAlignment) {
    HugepageArena* arena = arena_init();
    void* ptr = arena_alloc(arena, 64, 8192); // greater than 4096
    EXPECT_EQ(ptr, nullptr);
    arena_free(arena);
}

// arena_alloc new chunk allocation
TEST(ArenaAllocTest, ChunkBoundaryAllocation) {
    HugepageArena* arena = arena_init();
    size_t large_size = K_HUGEPAGE_SIZE - 64;
    void* ptr1 = arena_alloc(arena, large_size, 8);
    ASSERT_NE(ptr1, nullptr);
    void* ptr2 = arena_alloc(arena, 128, 8);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_NE(ptr1, ptr2);
    ASSERT_NE(arena->head, nullptr);
    EXPECT_NE(arena->head->next, nullptr); // new chunk should exist
    arena_free(arena);
}

// arena_free test
TEST(ArenaFreeTest, FreeWithoutCrash) {
    HugepageArena* arena = arena_init();
    void* ptr = arena_alloc(arena, 256, 8);
    ASSERT_NE(ptr, nullptr);
    arena_free(arena);
    SUCCEED(); // no crash means pass
}
