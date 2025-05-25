/**
 *  @file    merkle_concepts.hh
 *  @brief   Custom concepts for implementing the Merkle tree
 *  @author  https://github.com/gdaneek
 *  @date    30.05.2025
 *  @version 1.0
 *  @see https://github.com/gdaneek/merkle-tree
 */


#pragma once

#include <concepts>
#include <type_traits>
#include <iterator>


template<typename T>
concept PODType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

template<typename T>
concept HasDataAndSize = requires(T t) {
    { t.size() } -> std::integral;
    { t.data() } -> std::convertible_to<void*>;
};


/**
 * @brief requires that the container is contiguous
 * @note continuity means that the container is stored in memory without any padding or breaks
 */
template<typename T>
concept ContiguousContainer = PODType<T> || HasDataAndSize<T>;


/**
 * @brief requires that the container is iterable
 */
template <typename T>
concept Iterable = requires(T t) {
    { std::begin(t) } -> std::input_iterator;
    { std::end(t) } -> std::sentinel_for<decltype(std::begin(t))>;
};



/**
 * @brief requires that the container is indexable
 */
template<typename T>
concept Indexable = requires(T t, size_t i) {
    { t[i] };
};
