// adapted to this project by D. Lemire, from https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
// This uses mum hashing.
// 32 bit version
#include <stdint.h>
// state for wyhash64
uint32_t wyhash32_x; /* The state can be seeded with any value. */

// call wyhash64_seed before calling wyhash64
static inline void wyhash32_seed(uint64_t seed) { wyhash32_x = seed; }

static inline uint32_t wyhash32_stateless(uint32_t* seed) {
    *seed += 0xe120fc15;
    uint64_t tmp;
    tmp = (uint64_t)*seed * 0x4a39b70d;
    uint32_t m1 = (tmp >> 32) ^ tmp;
    tmp = (uint64_t)m1 * 0x12fad5c9;
    uint32_t m2 = (tmp >> 32) ^ tmp;
    return m2;
}

static inline uint32_t wyhash32(void) { return wyhash32_stateless(&wyhash32_x); }


static inline float wyhash32_float(float min, float max)
{
    return ((float)wyhash32() / (float)(0x7FFFFFFF)) * (max - min) + min;
}