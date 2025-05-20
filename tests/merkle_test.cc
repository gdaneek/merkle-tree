/**
 * @file merkle_test.cc
 * @brief   Tests for Merkle trees
 * @author  https://github.com/gdaneek
 * @date    30.05.2025
 * @version 1.0-beta
 * @see https://github.com/gdaneek/MerkleTree.git
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "merkle.hpp"
#include <algorithm> // for std::equal
#include <array>
#include <string>
#include <vector>


namespace fs_tree_tests {

TEST_SUITE("MerkleTree fixed size (FS) implementation tests") {

    // simplest hash function only for tests
    constexpr auto hash(const uint8_t * const bytes, const size_t n) {
        union { uint64_t v; uint8_t bytes[sizeof(v)]; } hash{};
        for(size_t i{}; i < n; ++i)
            hash.v = (hash.v * 31) + bytes[i];

        return std::to_array(hash.bytes); // std::array<uint8_t, 8>
    }

    template<typename T>
    auto operator+(T&& lhs, T&& rhs) {
        return concat_bytes(lhs, rhs);
    }


    TEST_CASE("[build] single node") {
        auto tree = merkle::make_fs_tree<1>(hash, std::vector<std::string>{"one"});

        REQUIRE(tree.height() == 0);
        REQUIRE(tree.root() == tree.node_hash((std::string)"one"));
    }


    TEST_CASE("[build] two nodes, simplest tree with non-zero height") {
        std::vector<std::string> d = {"lhs", "rhs"};
        auto tree = merkle::make_fs_tree<2>(hash, d);

        REQUIRE(tree.height() == 1);
        REQUIRE(tree.root() == tree.node_hash(tree.leaf_hash(d[0]) + tree.leaf_hash((d[1]))));
    }


    TEST_CASE("[build] five nodes, tree with node additions") {
        std::vector<std::string> d = {"first", "second", "third", "fourth", "fifth"};
        auto tree = merkle::make_fs_tree<5>(hash, d);

        REQUIRE(tree.height() == 3);    // ceil(log2(5)) == 3

        auto lh{[&](auto&& x){ return tree.leaf_hash(x); }};
        auto nh{[&](auto&& x){ return tree.node_hash(x); }};

        std::vector<std::array<uint8_t, sizeof(uint64_t)>> l3 = {lh(d[0]), lh(d[1]), lh(d[2]), lh(d[3]), lh(d[4]), lh(d[4])};
        decltype(l3) l2 = {nh(l3[0] + l3[1]), nh(l3[2] + l3[3]), nh(l3[4] + l3[5]), nh(l3[4] + l3[5])};
        decltype(l2) l1 = {nh(l2[0] + l2[1]), nh(l2[2] + l2[3])};

        REQUIRE(tree.root() == nh(l1[0] + l1[1]));
    }


    TEST_CASE("[verify] Valid nodes verification methods") {
        auto d = std::vector<std::string>{"first", "second"};
        auto tree = merkle::make_fs_tree<2>(hash, d);

        REQUIRE((tree.verify(d[0]) && tree.verify(d[1]) && tree.has(d[1])) == true);
        REQUIRE((tree.has((std::string)"third")) == false);
    }


    TEST_CASE("[proof] get proof (simple v)") {
        std::vector<std::string> d = {"lhs", "rhs"};
        auto tree = merkle::make_fs_tree<2>(hash, d);

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
        auto tree = merkle::make_fs_tree<5>(hash, d);

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

}

};
