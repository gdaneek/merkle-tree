/**
 * @file merkle_test.cc
 * @brief   Tests for Merkle trees
 * @author  https://github.com/gdaneek
 * @date    30.05.2025
 * @version 1.1
 * @see https://github.com/gdaneek/MerkleTree.git
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "merkle.hpp"
#include <algorithm>
#include <array>
#include <string>
#include <vector>


using namespace merkle;

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr) {
    if (N == 0)
        return os;

    os << std::hex << arr[0];
    std::for_each(arr.begin() + 1, arr.end(), [&os](const T& elem) {os << (int)elem;});

    return os;
}


namespace fs_tree_tests {

TEST_SUITE("MerkleTree fixed size (FS) implementation tests") {


    struct Hasher {
        using value_type = typename std::array<char, 8>;
        constexpr auto operator()(auto&& cont) const -> value_type  {
            union { uint64_t v; char bytes[sizeof(v)]; } hash{};
            for(auto&& x : cont)
                hash.v = (hash.v * 31) + x;

            return std::to_array(hash.bytes); // std::array<char, 8>
        }
    };


    template<typename T>
    auto operator+(T&& lhs, T&& rhs) {
        return bconcat::UnifiedConcatenator::concat(lhs, rhs);
    }


    TEST_CASE("[build] single node") {
        FixedSizeTree<Hasher, 1> tree(std::vector<std::string>{"one"});

        REQUIRE(tree.height() == 0);
        REQUIRE(tree.root() == tree.node_hash((std::string)"one"));
    }



    TEST_CASE("[build] two nodes, simplest tree with non-zero height") {
        std::vector<std::string> d = {"lhs", "rhs"};
        FixedSizeTree<Hasher, 2> tree(d);

        REQUIRE(tree.height() == 1);
        REQUIRE(tree.root() == tree.node_hash(tree.leaf_hash(d[0]) + tree.leaf_hash((d[1]))));
    }


    TEST_CASE("[build] five nodes, tree with node additions while build") {
        std::vector<std::string> d = {"first", "second", "third", "fourth", "fifth"};

        FixedSizeTree<Hasher, 5> tree(d);

        REQUIRE(tree.height() == 3);    // ceil(log2(5)) == 3

        auto lh{[&](auto&& x){ return tree.leaf_hash(x); }};
        auto nh{[&](auto&& x){ return tree.node_hash(x); }};

        std::vector<std::array<char, sizeof(uint64_t)>> l3 = {lh(d[0]), lh(d[1]), lh(d[2]), lh(d[3]), lh(d[4]), lh(d[4])};
        decltype(l3) l2 = {nh(l3[0] + l3[1]), nh(l3[2] + l3[3]), nh(l3[4] + l3[5]), nh(l3[4] + l3[5])};
        decltype(l2) l1 = {nh(l2[0] + l2[1]), nh(l2[2] + l2[3])};

        REQUIRE(tree.root() == nh(l1[0] + l1[1]));
    }


    TEST_CASE("[verify] Valid nodes verification methods") {
        auto d = std::vector<std::string>{"first", "second"};
        FixedSizeTree<Hasher, 2> tree(d);

        REQUIRE((tree.verify(d[0]) && tree.verify(d[1]) && tree.has(d[1])) == true);
        REQUIRE((tree.has((std::string)"third")) == false);
    }


    TEST_CASE("[proof] get proof (simple v)") {
        std::vector<std::string> d = {"lhs", "rhs"};
        FixedSizeTree<Hasher, 2> tree(d);

        auto v = tree.leaf_hash(d[0]);

        auto [initial, proof] = tree.get_proof((std::string)"lhs");
        auto back = proof.back();

        REQUIRE(initial == v);

        for(auto i = 0;i < proof.size()-1;++i) {
            auto [curr_hash, dir] = proof[i];
            v = dir? tree.node_hash(curr_hash + v) : tree.node_hash(v + curr_hash);
        }

        REQUIRE(v == back.first);
    }


    TEST_CASE("[proof] get proof (hard v)") {
        std::vector<std::string> d = {"first", "second", "third", "fourth", "fifth"};
        FixedSizeTree<Hasher, 5> tree(d);

        for(auto&& s : d) {
            auto v = tree.leaf_hash(s);

            auto [initial, proof] = tree.get_proof(s);
            auto back = proof.back();

            REQUIRE(initial == v);

            for(auto i = 0;i < proof.size()-1;++i) {
                auto [curr_hash, dir] = proof[i];
                v = dir? tree.node_hash(curr_hash + v) : tree.node_hash(v + curr_hash);
            }

           REQUIRE(v == back.first);
        }
    }

}};
