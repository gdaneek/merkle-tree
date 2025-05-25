/**
 *  @file    merkle_utils.hh
 *  @brief   auxiliary functions for implementing the Merkle tree
 *  @author  https://github.com/gdaneek
 *  @date    30.05.2025
 *  @version 1.0
 *  @see https://github.com/gdaneek/merkle-tree
 */

#pragma once

#include <array>
#include <cstring>
#include <cstdint>

/**
 * @brief fastest implementation of the base 2 integer algorithm
 * @param x value for which the logarithm is calculated
 * @return [log2(x)] (whole part of the logarithm)
 */
template<typename T>
constexpr inline T ilog2(T x) {
    return x? 63 - __builtin_clzll(x) : -1;
}

/**
 * @brief finds nearest (equal or greater) even number
 */
template<typename T>
constexpr inline T round_to_even(T in) {
    return in + (in & 1);
}


 /**
  * @brief performs concatenation of any POD containers
 */
constexpr auto concat_bytes(auto&&... args) {
    char out[(sizeof(args) + ...)];
    uint64_t s{};
    ((memcpy(out + s, &args, sizeof(args)), s += sizeof(args)), ...);

    return std::to_array(out);
}
