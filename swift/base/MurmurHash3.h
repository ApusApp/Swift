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

// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef __SWIFT_BASE_MURMUR_HASH3_H__
#define __SWIFT_BASE_MURMUR_HASH3_H__

#include <stdint.h>

namespace swift {

    void MurmurHash3_x86_32 (const void *key, int len, uint32_t seed, void *out);

    void MurmurHash3_x86_128 (const void *key, int len, uint32_t seed, void *out);

    void MurmurHash3_x64_128 (const void *key, int len, uint32_t seed, void *out);
}

#endif // __SWIFT_BASE_MURMUR_HASH3_H__
