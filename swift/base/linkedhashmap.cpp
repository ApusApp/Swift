/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "swift/base/linkedhashmap.h"
#include <sys/mman.h>

namespace swift {
namespace detail {
    void* MapAlloc (size_t size)
    {
        assert (size > 0 && size <= std::numeric_limits<size_t>::max () / 2);
        void* ptr = ::mmap (0, sizeof(size) + size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (MAP_FAILED == ptr) {
            throw std::bad_alloc ();
        }

        *(size_t*)ptr = size;
        return (char*)ptr + sizeof(size);
    }

    void MapFree (void* ptr)
    {
        if (ptr) {
            size_t size = *((size_t*)ptr - 1);
            ::munmap ((char*)ptr - sizeof(size), sizeof(size) + size);
        }
    }
} // namespace swift
} // namespace swift
