/**
 *  @file    merkle.hpp
 *  @brief   Several implementations of the Merkle tree
 *  @author  https://github.com/gdaneek
 *  @date    30.05.2025
 *  @version 1.1
 *  @see https://github.com/gdaneek/merkle-tree
 */


#pragma once

#include <iostream> // for << operator

#include "merkle_utils.hpp"
#include "bytes_concat.hpp"

namespace merkle {

    /**
     * @brief calculates the number of hashes in a Merkle tree
     * @details
     * calculates the number of hashes in a Merkle trees, created by cloning the last node
     * at odd-length levels based on the number of input elements
     * @param leafs_n number of input elements
     * @return number of hashes in a Merkle tree
     */
    constexpr inline auto calc_tree_size(uint64_t leafs_n) {
        uint64_t s{1};
        for(;leafs_n > 1;leafs_n >>= 1)
            s += (leafs_n += leafs_n & 1);

        return s;
    }


    /**
     * @brief a base template class for building Merkle trees based on CRTP
     * @details
     * Class defines the required interface using concepts and
     * provides auxiliary methods for calculating the size, height, leaf search, etc..., through inheritance.
     * All trees inheriting this automatically have some principles for ensuring security, such as double hashing with salt
     * (protection against Second Preimage / Length Extension Attacks if the hash function is based on Merkle–Damgard)
     * and two generalized algorithms for building a tree (todo status).
     * @tparam Derived concrete implementation of the Merkle tree
     * @tparam HashFunc type of hash function
     */
    template<typename Derived, typename Hasher, typename Concatenator = bconcat::UnifiedConcatenator>
    class TreeBase {

        Hasher m_hash; ///<  hash function
        Concatenator m_concat; ///< hashes concatenation func

    protected:

        /**
         * @brief default constructor
         * @param h link to the hash function that will be used to build the tree
         */
        constexpr explicit TreeBase(Hasher h, Concatenator c)
        : m_hash{h}, m_concat{c} {}

        constexpr TreeBase()
        : m_hash{}, m_concat{} {}


        /**
         * @brief a function that finds a leaf of the Merkle tree in the hash container corresponding to the input data
         * @param data any data that can be hashed
         * @return position where the leaf hash corresponding to the `data` arg is located
         * @note if the transmitted data was not used when creating tree, it returns -1
         * @note O(N) complexity where N is equal to the number of leaves in the tree
         */
        template<typename... Args>
        constexpr auto find_leaf(Args&&... data) const {
            // TODO: it not constexpr because using reinterpret_cast. need refactor

            auto lhash = leaf_hash(std::forward<Args>(data)...);
            auto leafs_n = reinterpret_cast<const Derived*>(this)->get_leafs_n();
            auto idata = reinterpret_cast<const Derived*>(this)->data();

            if constexpr (Iterable<decltype(idata)>) {
                for(auto it = idata.begin();it != idata.end();it++)
                    if(*it == lhash)
                        return it;

                return idata.end();
            } else {
                for(size_t i{};i < leafs_n;++i)
                    if(idata[i] == lhash)
                        return i;

                return (size_t)-1;
            }
        }


        template<typename... Args>
        constexpr auto hash(Args&&... args) const {
            return m_hash(std::forward<Args>(args)...);
        }


        template<typename... Args>
        constexpr auto concat(Args&&... args) const {
            return m_concat(std::forward<Args>(args)...);
        }


        public:

        /**
         * @brief calculates the hash of a tree leaf
         * @details
         * uses double leaf hashing to protect against Length Extension Attack
         * @param args parameters for calling the hash function (see hash method overloads)
         *
         */
        template<typename... Args>
        constexpr auto leaf_hash(Args&&... args) const {
            constexpr int leaf_salt = 0x00; // for tests only
            // TODO: can be well optimized if you spread the salt in advance
            return hash(m_concat(leaf_salt, std::forward<Args>(args)...));
        }


        /**
         * @brief calculates the hash of a tree node
         * @param args parameters for calling the hash function (see hash method overloads)
         */
        template<typename... Args>
        constexpr auto node_hash(Args&&... args) const {
            constexpr int node_salt = 0x01; // for tests only
            // TODO: can be well optimized if you spread the salt in advance
            return hash(m_concat(node_salt, std::forward<Args>(args)...));
        }


        /**
         * @brief a function that allows you to find out the height of any Merkle tree by the number of leaves
         * @param leafs_n number of leaves in Merkle tree
         * @return tree height
         * @warning If the tree has zero or one (only root) nodes, then the height is zero
         */
        static constexpr size_t height(const size_t leafs_n) {
            return leafs_n? (ilog2(leafs_n) + (bool)(__builtin_popcount(leafs_n) - 1)) : 0u;
        }


         /**
         * @brief function that calculates the height of a current Merkle tree
         * @return tree height
         * @warning If the tree has zero or one (only root) nodes, then the height is zero
         */
        size_t height() const {
            return height(reinterpret_cast<const Derived*>(this)->get_leafs_n());
        }


        static constexpr size_t size(const size_t leafs_n) {
            return calc_tree_size(leafs_n);
        }

        /**
         * @brief function that calculates the number of elements (hashes) for a current Merkle tree
         * @return number of hashes in tree
         */
        constexpr size_t size() const {
            return this->size(reinterpret_cast<const Derived*>(this)->get_leafs_n());
        }


        /**
         * @brief checks whether the data block was used when creating the Merkle tree
         * @details
         * If the method returns some index that is not equal to the maximum allowed size_t value,
         * therefore, the input data has a corresponding leaf in the tree
         * @param data any data that can be hashed
         */
        template<typename... Args>
        constexpr auto verify(Args&&... data) const {
            return (find_leaf(std::forward<Args>(data)...) != (size_t)-1);
        }


        /**
         * @brief an alias for verify
         */
        template<typename... Args>
        constexpr auto has(Args&&... data) const {
            return verify(std::forward<Args>(data)...);
        }
    };


    /**
     * @brief General class of Merkle trees calculated at the compilation stage and trees with a size calculated at the compilation stage
     * @details
     * built on the basis of std::array to ensure maximum performance.
     * The elements of each layer are laid out in a continuous section of memory, starting with N leaves, then N / 2 nodes, etc.
     * Due to this stacking, an implicit concatenation of neighboring hashes occurs, and when calculating a node of the next level,
     * exactly 0 operations are spent on adding its child elements.
     * By default, preference is given to the construction algorithm with copying the last node on layers with an odd size,
     * since the construction algorithm is much simpler for it, and the additional memory consumption is insignificant.
     * @tparam Hasher type of hash function
     * @tparam LEAFS_N  the number of leaves in the tree calculated at the compilation stage
     */
    template<typename Hasher, uint64_t LEAFS_N, typename Hash = Hasher::value_type, typename Concatenator = bconcat::UnifiedConcatenator>
    class FixedSizeTree : public TreeBase<FixedSizeTree<Hasher, LEAFS_N, Hash, Concatenator>, Hasher, Concatenator> {

        // TODO: Custom concatenator with support for implicit concatenation while build like in v1.0

        using Base = TreeBase<FixedSizeTree<Hasher, LEAFS_N, Hash, Concatenator>, Hasher, Concatenator>;

        inline static constexpr auto SIZE = calc_tree_size(LEAFS_N); ///< number of hashes in tree
        std::array<Hash, SIZE> m_data; ///< flattened hashes tree

    protected:


        /**
         * @brief finds a pointer to a tree layer by index and its size
         * @param idx layer index (0 for the root, 1..N for the following)
         * @return pointer to the beginning of the layer and its length (num of hashes)
         * @note index of the last layer is equal to the height value
         * O(logN) complexity where N is equal to the number of hashes in the tree
         */
        constexpr auto get_layer(const size_t idx) const { // 0 for root
            size_t offset{}, n{LEAFS_N + (LEAFS_N & 1)}, height{Base::height(LEAFS_N)};
            for(auto i = height-idx;i;--i, (n>>=1)+=n&1)
                offset += n;

            return std::make_pair(m_data.data() + offset, n - !idx);
        }

    public:

        /**
         * @brief constructor for immediate creation of a tree from a data container
         * @param _hash link to the hash function that will be used to build the tree
         * @param _data data container
         * @note the num of elements in the container should be equal to LEAFS_N or LEAFS_N-1
         * @warning you can use containers with fewer than LEAFS_N elements, but in this case some methods will give the wrong answer.
         * Using a container with a size larger than LEAFS_N leads to the building of a tree based only on the first LEAFS_N elements.
         */
        template<typename T>
        constexpr FixedSizeTree(Hasher _h, Concatenator _c, T&& _data) : Base::TreeBase(_h, _c)  {
            build(std::forward<T>(_data));
        }

        template<typename T>
        constexpr explicit FixedSizeTree(T&& _data) : Base::TreeBase()  {
            build(std::forward<T>(_data));
        }

        constexpr explicit FixedSizeTree(Hasher _h, Concatenator _c): Base::TreeBase(_h, _c) {}
        constexpr FixedSizeTree() : Base() {}


        /**
         * @brief build a tree based on a data container
         * @param ccont data container
         * @return this object
         * @note O(N) complexity where N is equal to the number of hashes in the tree
         */
         constexpr auto build(auto&& ccont) { // requires ccont.size() == SIZE

            if constexpr (LEAFS_N == 1) {
                m_data[0] = this->node_hash(*ccont.begin());
                return *this;
            }

            size_t it{};
            for(auto&& x : ccont) // i don't want use std::transform
                m_data[it++] = this->leaf_hash(x);


            for(size_t t{}, l{}, r{LEAFS_N};l < r-1;t = l, l = r, r += ((r - t) >> 1)) {
                if(r & 1) m_data[r++] = m_data[r - 1];
                for(uint64_t i = 0;i < (r-l)>>1; i++)
                    m_data[r + i] = this->node_hash(m_data[(i<<1) + l], m_data[(i<<1) + l + 1]); // implicit concat available

            }

            return *this;
        }


        /**
         * @brief creates a proof of inclusion of some data in the tree
         * @param data input for which the proof is being created
         * @return pair from the hash of the leaf and an array of hashes from all levels, proving the inclusion of data in the tree
         * @note if the data was not used to build the tree, an object consisting of default values will be returned.
         * For the data used in the construction, the first argument will always match the hash of the desired tree leaf.
         * @note O(logN) complexity where N is equal to the number of hashes in the tree
         */
        constexpr auto get_proof(auto&& data) const {
            constexpr auto height = Base::height(LEAFS_N);
            std::array<std::pair<Hash, bool>, height + 1> proof{};                  // because for 0 ... 1 levels height == 1 but proof vals num = 2

            if constexpr (LEAFS_N == 1)
                return this->node_hash(data) == m_data[0]? decltype(proof){std::make_pair(m_data[0], bool{})} : proof;

            auto idx = this->find_leaf(data);
            if(idx == (size_t)-1)
                return std::make_pair(Hash{}, proof);   // empty array

            auto initial = m_data[idx];

            for(size_t i{};i < height;++i, idx >>= 1)
                proof[i] = std::make_pair(get_layer(height - i).first[idx + (idx & 1? -1 : 1)], (bool)(idx & 1));

            proof[proof.size() - 1] = std::make_pair(this->root(), bool{});

            return std::make_pair(initial, proof);
        }

        /**
         * ...
         * @note O(logN) complexity where N is equal to the number of hashes in the tree
         */
        constexpr auto verify_proof(auto&& data, auto&& proof) {  // proof - ...<std::pair<Hash, bool>>
            auto curr_hash = this->leaf_hash(data);
            auto supposed_root = proof(proof.size() - 1).first;
            for(auto i = 0;i < proof.size() - 1;++i)
                curr_hash = proof[i].second?  this->node_hash(proof[i].first, curr_hash)
                                           :  this->node_hash(curr_hash, proof[i].first);

            return (curr_hash == supposed_root);
        }


        /**
         * @brief root of the Merkle tree
         * @return last (topmost) hash
         */
        constexpr auto root() const {
            return m_data[SIZE - 1];
        }


        /**
         * @brief current num of leaves in the tree
         * @note For fixed size class of trees, the function can be static,
         * since it depends only on the class parameters, but not on a specific object
         */
        static constexpr auto get_leafs_n() {
            return LEAFS_N;
        }


        /**
         * @brief returns a pointer (or iterator) to the hash tree
         * @return pointer to the beginning of the tree (the leftmost element of the lower layer).
         * The pointer is traversed from left to right from bottom to top. total number of hashes available
         */
        constexpr auto data() const {
            return m_data.data();
        }


        friend std::ostream& operator<<(std::ostream& os, FixedSizeTree<Hasher, LEAFS_N, Hash, Concatenator>& tree) {
            os << "Merkle tree:\n";
            for(auto i = 0;i <= tree.height();++i) {
                auto [ldata, lsz] = tree.get_layer(tree.height() - i);
                os << "\nLayer " << (tree.height() - i) << " (size = " << lsz << "):\n";
                for(auto j = 0;j < lsz;++j)
                    os << ldata[j] << "\n";
            }

             return os;
        }
    };


    // TODO: Dymamic resizeble tree

};


