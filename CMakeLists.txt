cmake_minimum_required(VERSION 3.20)

project(merkle_tree)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(merkle_test tests/merkle_test.cc)
target_include_directories(merkle_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
#[[
add_executable(example example/fs_tree.cc)
target_include_directories(example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)]]

enable_testing()
add_test(NAME merkle_test COMMAND merkle_test)
target_compile_options(merkle_test PRIVATE -std=c++20 -O3)

add_library(merkletree INTERFACE)
target_include_directories(merkletree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
