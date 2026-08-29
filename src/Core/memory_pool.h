#pragma once

#include <array>
#include <cstddef>
#include <memory>

namespace exchange::core
{
    template <typename T, std::size_t N>

    class MemoryPool
    {
        static_assert(N > 0, "MemoryPool size must be greater than zero.");
        static_assert(sizeof(T) >= sizeof(void *), "Object storage must fit an intrusive free-list pointer.");

        struct alignas((alignof(T) > 64U) ? alignof(T) : 64U) Slot
        {
            std::array<std::byte, sizeof(T)> storage{};
        };

        struct FreeNode
        {
            FreeNode *next;
        };

    public:
        MemoryPool() : slots_(std::make_unique<Slot[]>(N))
        {
            initializeFreeList();
        }

        MemoryPool(const MemoryPool &) = delete;
        MemoryPool &operator=(const MemoryPool &) = delete;
        MemoryPool(MemoryPool &&) = delete;
        MemoryPool &operator=(MemoryPool &&) = delete;

        T* allocate() noexcept
        {
            if(freeListHead == nullptr) [[unlikely]]
            {
                return nullptr; // Pool exhausted
            }
            auto *node = freeListHead;
            freeListHead = freeListHead->next;
            return reinterpret_cast<T *>(node);
        }

    private:
        void initializeFree() noexcept
        {
            freeListHead = nullptr;
            for (std::size_t i = 0; i < N; ++i)
            {
                auto *node = reinterpret_cast<FreeNode *>(&slots_[i].storage.data());
                node->next = freeListHead;
                freeListHead = node;
            }
        }

        std::unique_ptr<Slot[]> slots_;
        FreeNode *freeListHead{nullptr};
    };
}