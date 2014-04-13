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
