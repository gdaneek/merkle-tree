
#include "merkle.hpp"
#include <string>
#include <vector>


struct Hasher {
    using value_type = uint64_t;
    constexpr auto operator()(auto&& cont) const -> value_type  {
        uint64_t hash{};
        for(auto&& x : cont)
            hash = (hash * 31) + x;

        return hash;
    }
};

using namespace merkle;

int main() {

    std::vector<std::string> fnames = {"passwords.db", "users.txt", "raw_data.bin"};

    FixedSizeTree<Hasher, 3> tree; /* pass also fnames to immediate build*/
    tree.build(fnames);


    std::cout << std::hex << "0x" << tree.root() << std::endl;

    return 0;

}
