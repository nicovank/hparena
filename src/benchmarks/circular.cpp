#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory_resource>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <argparse/argparse.hpp>
#include <benchmark/benchmark.h>

#include <hparena/hparena.hpp>

namespace {
struct Node {
    Node* next = nullptr;
    std::array<std::uint8_t, 4128 - sizeof(Node*)> padding;

    Node() = default;
};

static_assert(sizeof(Node) >= 4096);
} // namespace

int main(int argc, char** argv) {
    auto program = argparse::ArgumentParser("circular", "",
                                            argparse::default_arguments::help);
    program.add_argument("-n", "--number-objects")
        .required()
        .help("the number of objects to be allocated")
        .metavar("N")
        .scan<'u', std::uint64_t>();
    program.add_argument("-i", "--iterations")
        .required()
        .help("the number of iterations over the entire allocated population")
        .metavar("N")
        .scan<'u', std::uint64_t>();
    program.add_argument("--memory-resource")
        .help("which PMR memory resource to use")
        .default_value("new-delete")
        .choices("new-delete", "monotonic-buffer", "HugepageArena")
        .metavar("POLICY");
    program.add_argument("--no-shuffle")
        .help("disable shuffling the cycle")
        .default_value(false)
        .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(EXIT_FAILURE);
    }

    const auto nObjects = program.get<std::uint64_t>("--number-objects");
    const auto policy = program.get<std::string>("--memory-resource");
    const auto shuffle = !program.get<bool>("--no-shuffle");
    auto iterations = program.get<std::uint64_t>("--iterations");

    std::cout << "policy            : " << policy << std::endl;
    std::cout << "number of objects : " << nObjects << std::endl;
    std::cout << "iterations        : " << iterations << std::endl;
    std::cout << "shuffle           : " << (shuffle ? "yes" : "no")
              << std::endl;

    std::pmr::memory_resource* resource;
    if (policy == "new-delete") {
        resource = std::pmr::new_delete_resource();
    } else if (policy == "monotonic-buffer") {
        resource = new std::pmr::monotonic_buffer_resource();
    } else if (policy == "HugepageArena") {
        resource = new hparena::HugepageArena<
            hparena::HugepageAllocationFailurePolicy::ThrowException>();
    } else {
        std::unreachable();
    }

    const std::pmr::polymorphic_allocator<Node> allocator(resource);

    // Nest so the vector destructor runs before the resource destructor...
    {
        auto objects = std::pmr::vector<Node>(allocator);
        objects.reserve(nObjects);

        for (std::size_t i = 0; i < nObjects; ++i) {
            objects.emplace_back();
        }

        if (shuffle) {
            std::cout << "Shuffling iteration order..." << std::endl;
            std::shuffle(objects.begin(), objects.end(), std::random_device{});
        }

        std::cout << "Setting up cycle..." << std::endl;
        objects.back().next = objects.data();
        for (std::size_t i = 0; i < nObjects - 1; ++i) {
            objects[i].next = &objects[i + 1];
        }
        Node* n = objects.data();

        std::cout << "Iterating..." << std::endl;

        const auto start = std::chrono::high_resolution_clock::now();
        while (--iterations > 0) {
            n = n->next;
        }
        benchmark::DoNotOptimize(n);
        const auto end = std::chrono::high_resolution_clock::now();

        const auto elapsed_ms
            = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();
        std::cout << "Done. Time elapsed: " << elapsed_ms << " ms."
                  << std::endl;
    }

    if (policy == "monotonic-buffer" || policy == "HugepageArena") {
        delete resource;
    }
}
