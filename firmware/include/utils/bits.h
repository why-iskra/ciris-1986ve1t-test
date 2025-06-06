//
// Created by whyiskra on 2025-05-24.
//

#pragma once

#define addr32(x) (*((uint32_t *) (x)))

#define set_bits(c, n, m, v) ((c) = ((__typeof__(c)) (((c) & ~(((__typeof__(c)) (m)) << (n))) | (__typeof__(c)) ((__typeof__(c)) (v) << (n)))))
#define set_bit(c, n, v)     set_bits(c, n, 1, (v) == 0 ? 0 : 1)

#define get_bits(c, n, m) ((((c) >> (n)) & (m)) == (m))
#define get_bit(c, n)     ((((c) >> (n)) & (1)) == (1))
