cmake_minimum_required(VERSION 3.14)

project(
        rbtree
        DESCRIPTION "Red-black search tree implementation"
        LANGUAGES CXX
        )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

option(DEBUG_FLAGS "Use debug compilation flags" OFF)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(DEBUG_FLAGS OFF)
endif()

if (DEBUG_FLAGS)
        set(
        library_compile_options 
        -O0 -g -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual 
        -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 
        -Winit-self -Wmissing-declarations -Wmissing-include-dirs 
        -Wold-style-cast -Woverloaded-virtual -Wredundant-decls 
        -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 
        -Wswitch-default -Wundef -Wunused -Wshadow 
        )
else()
        set(library_compile_options -O2)
endif()

add_library(rbtree INTERFACE)
target_include_directories(
        rbtree INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:inc
        )
target_compile_features(rbtree INTERFACE cxx_std_20)
target_compile_options(
        rbtree INTERFACE 
        $<BUILD_INTERFACE:${library_compile_options}>
        )

install(DIRECTORY inc DESTINATION inc)

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)
if (NOT IS_SUBPROJECT)
        enable_testing()
        add_subdirectory(tests)
endif()
