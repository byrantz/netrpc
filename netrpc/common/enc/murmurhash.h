#ifndef NETRPC_COMMON_ENCO_MUR_H
#define NETRPC_COMMON_ENCO_MUR_H
#include <stdint.h>

namespace netrpc {
/*
 * MurMurHash算法，是非加密HASH算法，性能很高，
 * 比传统的CRC32,MD5，SHA-1（这两个算法都是加密HASH算法，复杂度本身就很高，带来的性能上的损害也不可避免）
 * 等HASH算法要快很多，而且这个算法的碰撞率很低.
 * http://murmurhash.googlepages.com/
 * 参数seed：种子，最好用一个质数
 */
uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed = 17U);

}

#endif