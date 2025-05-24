
#include "merkle.hpp"
#include <string>
#include <vector>

int main() {

    auto hasher{[&](const char* c, size_t n) { // tree v1.0 compatible hash lambda for strings
        std::hash<std::string> hasher;
        return hasher(std::string(c));
    }};


    std::vector<std::string> fnames = {"passwords.db", "users.txt", "raw_data.bin"};

    auto tree = merkle::make_fs_tree<5>(hasher); /* pass also fname to immediate build*/
    tree.build(fnames);

    std::cout << std::hex << tree.root() << std::endl;

    return 0;

}
