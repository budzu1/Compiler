#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <cstddef>
#include <stack>

class MemoryManager {
    long long nextOffset;
    std::stack<long long> freeStack;

public:
    MemoryManager() : nextOffset(1)
    {}

    long long allocate(size_t size=1) {
        long long base = nextOffset;
        nextOffset += size;
        return base;
    }


    long long getNextAddress() {
        return nextOffset;
    }

    void memSetNextAddress(long long addr) {
        nextOffset = addr;
    }
};

#endif
