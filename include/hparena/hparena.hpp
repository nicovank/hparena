#ifndef HUGEPAGE_ARENA_HPP
#define HUGEPAGE_ARENA_HPP

#include <cassert>
#include <cstddef>
#include <deque>
#include <memory_resource>
#include <stdexcept>
#include <utility>

#include <sys/mman.h>

namespace {
constexpr std::size_t kHugepageSize = 1 << 21;

template <typename T>
constexpr T alignUp(T size, T alignment) requires std::is_unsigned_v<T> {
    return (size + alignment - 1) & ~(alignment - 1);
}
} // namespace

// // TODO.
// // ASAN: If you have a custom allocation arena, the typical workflow would be
// to
// // poison the entire arena first, and then unpoison allocated chunks of
// memory
// // leaving poisoned redzones between them. The allocated chunks should start
// // with 8-aligned addresses.

namespace hparena {
enum class HugepageAllocationFailurePolicy {
    ThrowException,
    RevertToDefaultPages,
};

template <HugepageAllocationFailurePolicy Policy
          = HugepageAllocationFailurePolicy::RevertToDefaultPages>
class HugepageArena : public std::pmr::memory_resource {
  public:
    HugepageArena() = default;

    ~HugepageArena() override {
        // TODO: Maybe some pollution / address sanitizer markers in debug mode.
        for (const auto& [chunk, size] : chunks) {
            [[maybe_unused]] const int result = munmap(chunk, size);
            assert(result == 0);
        }
    }

    HugepageArena(const HugepageArena&) = delete;
    HugepageArena(HugepageArena&&) noexcept = default;
    HugepageArena& operator=(const HugepageArena&) = delete;
    HugepageArena& operator=(HugepageArena&&) noexcept = default;

  private:
    [[nodiscard]] void* do_allocate(std::size_t bytes,
                                    std::size_t alignment) override {
        if (alignment > kHugepageSize) [[unlikely]] {
            throw std::invalid_argument("do_allocate: alignment too big");
        }

        const std::uintptr_t alignedPtr = alignUp(currentBumpPtr, alignment);
        if (alignedPtr + bytes < currentChunkSize) [[likely]] {
            currentBumpPtr = alignedPtr + bytes;
            return reinterpret_cast<void*>(alignedPtr);
        }

        // Need to allocate a new chunk...
        // TODO: Maybe debug how much we wasted from the previous block.
        currentChunkSize
            = alignUp(std::max(currentChunkSize * 2, bytes), kHugepageSize);
        void* chunk = mmap(nullptr, currentChunkSize, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

        if constexpr (Policy
                      == HugepageAllocationFailurePolicy::
                          RevertToDefaultPages) {
            if (chunk == MAP_FAILED) {
                chunk = mmap(nullptr, currentChunkSize, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            }
        }

        if (chunk == MAP_FAILED) {
            throw std::bad_alloc();
        }

        chunks.emplace_back(chunk, currentChunkSize);
        currentBumpPtr = reinterpret_cast<std::uintptr_t>(chunk) + bytes;
        return chunk;
    }

    void do_deallocate([[maybe_unused]] void* p,
                       [[maybe_unused]] std::size_t bytes,
                       [[maybe_unused]] std::size_t alignment) override {
        // Blank.
    }

    [[nodiscard]] bool do_is_equal(
        const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

    std::deque<std::pair<void*, std::size_t>> chunks;
    std::uintptr_t currentBumpPtr = 0;
    std::size_t currentChunkSize = 0;
};
} // namespace hparena

#endif
