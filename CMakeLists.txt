cmake_minimum_required(VERSION 3.20)

project(MerkleTree)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(merkle_test tests/merkle_test.cc)
target_include_directories(merkle_test PUBLIC ./ include/)


enable_testing()
add_test(NAME merkle_test COMMAND merkle_test)

target_compile_options(merkle_test PRIVATE
        -std=c++20
        -O3
        -march=native
        -flto
        -fno-rtti
    )
