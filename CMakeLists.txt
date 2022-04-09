# TODO: consider lowering this version requirement to some lower v3.x
cmake_minimum_required(VERSION 3.17)

project(STOICAL VERSION 0.1.1 LANGUAGES C)
# this code looks quite old, let's not set a C standard version for now

add_subdirectory(src)
