/**
 *  @file    concepts.hh
 *  @brief   Custom concepts for implementing the Merkle tree
 *  @author  https://github.com/gdaneek
 *  @date    30.05.2025
 *  @version 1.0-beta
 *  @see https://github.com/gdaneek/MerkleTree.git
 */


#pragma once

#include <concepts>
#include <type_traits>
#include <iterator>

template<typename T>
concept HasDataMethod = requires(T t) {
    { t.data() } -> std::convertible_to<const unsigned char*>;
};


template<typename T>
concept ContiguousContainer = requires(T t) {
    { t.size() } -> std::integral;

};


template <typename T>
concept Iterable = requires(T t) {
    { std::begin(t) } -> std::input_iterator;
    { std::end(t) } -> std::sentinel_for<decltype(std::begin(t))>;
};


template<typename T>
concept Indexable = requires(T t, size_t i) {
    { t[i] };
    //{ t.size() } -> std::convertible_to<size_t>;
};
