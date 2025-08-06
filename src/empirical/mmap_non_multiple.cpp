#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <sys/mman.h>

int main() {
    constexpr std::size_t kSize = (1 << 21) + 1;
    void* allocation = mmap(nullptr, kSize, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (allocation == MAP_FAILED) {
        std::cout << "[ERROR] Failed to allocate hugepage: "
                  << std::strerror(errno) << std::endl;
        std::exit(1);
    }

    const int result = munmap(allocation, kSize);
    if (result != 0) {
        std::cout << "[ERROR] Failed to free hugepage: " << std::strerror(errno)
                  << std::endl;
        std::exit(1);
    }
}
