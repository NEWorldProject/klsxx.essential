/*
* Copyright (c) 2022 DWVoid and Infinideastudio Team
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <mutex>
#include <cstring>
#include "kls/hal/System.h"
#include "kls/essential/Memory.h"
#include "kls/essential/MemoryAVL.h"

namespace {
    // TODO(validate this)
    class block_host {
        // Sys Mem Management operations
        static auto reserve() noexcept {
#ifdef KLS_SYS_NTOS
            return VirtualAlloc(nullptr, g_reserved_address_space, MEM_RESERVE, PAGE_READWRITE);
#else
            return mmap(nullptr, g_reserved_address_space, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
#endif
        }

        void commit(const uint32_t block) const noexcept {
#ifdef KLS_SYS_NTOS
            VirtualAlloc(reinterpret_cast<LPVOID>(compute_base(block)), g_block_size, MEM_COMMIT, PAGE_READWRITE);
#else
            if (mprotect(reinterpret_cast<void *>(compute_base(block)), g_block_size, PROT_READ | PROT_WRITE) == -1) {
                puts(strerror(errno));
                fflush(stdout);
            }
#endif
        }

        void release(const uint32_t block) const noexcept {
#ifdef KLS_SYS_NTOS
            VirtualFree(reinterpret_cast<LPVOID>(compute_base(block)), g_block_size, MEM_DECOMMIT);
#else
            mprotect(reinterpret_cast<void *>(compute_base(block)), g_block_size, PROT_NONE);
            madvise(reinterpret_cast<void *>(compute_base(block)), g_block_size, MADV_DONTNEED);
#endif
        }

    public:
        block_host() noexcept
                : m_base_address(reinterpret_cast<uintptr_t>(reserve())),
                  m_start_address(block_align(m_base_address)), m_brk(0u), m_alloc(0u) {}

        // we do not need to cleanup anything as the OS will release them all on process termination

        uintptr_t rent() noexcept {
            const std::lock_guard lock(m_lock);
            return compute_base(alloc_id());
        }

        void free(uintptr_t ptr) noexcept {
            const std::lock_guard lock(m_lock);
            release_id((ptr - m_start_address) >> g_block_size_shl);
        }

        static block_host &instance() noexcept {
            static block_host instance{};
            return instance;
        }

    private:
        const uintptr_t m_base_address;
        const uintptr_t m_start_address;
        uint32_t m_brk, m_alloc;
        std::mutex m_lock;
        static constexpr uintptr_t g_block_size_shl = 22ull;
        static constexpr uintptr_t g_block_size = 1ull << g_block_size_shl;
        static constexpr uintptr_t g_reserved_address_space = 4ull << 28ull;

        // basic alignment computation
        [[nodiscard]] uintptr_t compute_base(const uint32_t block) const noexcept {
            return m_start_address + (block << g_block_size_shl);
        }

        static uintptr_t block_align(const uintptr_t in) noexcept {
            constexpr auto mask = (g_block_size - 1);
            return (in + mask) & (~mask);
        }

        kls::essential::MemoryAVL m_holes;

        uint32_t alloc_id() noexcept {
            if (const auto extract = m_holes.pop_front(); extract)
                return (extract - m_start_address) >> g_block_size_shl;
            if (m_brk == m_alloc) commit(m_alloc++);
            return m_brk++;
        }

        void release_id(const uint32_t id) noexcept {
            if (id + 1 == m_brk) {
                --m_brk;
                while (m_holes.pop_back_if(compute_base(m_brk - 1))) --m_brk;
                if (m_alloc > (m_brk + 5)) for (; m_alloc > m_brk; release(--m_alloc));
            } else m_holes.push(compute_base(id));
        }
    };
}

namespace kls::essential {
    uintptr_t rent_4m_block() noexcept { return block_host::instance().rent(); }

    void return_4m_block(uintptr_t block) noexcept { return block_host::instance().free(block); }
}
