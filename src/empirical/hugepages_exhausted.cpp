#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <sys/mman.h>

int main() {
    for (std::size_t i = 0;; ++i) {
        void* page = mmap(nullptr, 1 << 21, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (page == MAP_FAILED) {
            std::cout << "[ERROR] Failed to allocate hugepage #" << (i + 1)
                      << ": " << std::strerror(errno) << std::endl;
            std::exit(1);
        }
    }
}
