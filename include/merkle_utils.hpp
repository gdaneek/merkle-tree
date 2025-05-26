/**
 *  @file    merkle_utils.hpp
 *  @brief   Auxiliary functions for implementing the Merkle tree
 *  @author  https://github.com/gdaneek
 *  @date    30.05.2025
 *  @version 1.0
 *  @see https://github.com/gdaneek/merkle-tree
 */

#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <iterator>

#include "merkle_concepts.hpp"

namespace merkle {

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
    class ConcatPOD {
    public:
        constexpr auto operator()(auto&&... args) const {
            char out[(sizeof(args) + ...)];
            uint64_t s{};
            ((memcpy(out + s, &args, sizeof(args)), s += sizeof(args)), ...);

            return std::to_array(out);
        };
    };


    class UnifiedConcat {

        template<typename T> requires Iterable<T>
        void append(std::vector<char>& dst, T&& src) const {
            std::copy(std::begin(src), std::end(src), std::back_inserter(dst));
        }

        template<typename T> requires (!Iterable<T>)
        void append(std::vector<char>& dst, T&& src) const {
            auto curr_sz = dst.size();
            dst.resize(dst.size() + sizeof(src));

            memcpy(dst.data()+curr_sz , &src, sizeof(src));
        }

    public:
        template<typename... Args>
        constexpr auto operator()(Args&&... args) const {
            std::vector<char> bytes;
            ((append(bytes, std::forward<Args>(args))), ...);

            return bytes;
        }
    };

}
