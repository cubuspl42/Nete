cmake_minimum_required(VERSION 3.1)

project(Nete)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

set(SOURCES
    include/nete/tl/memory.h
    include/nete/tl/utility.h
    include/nete/tl/multivector.h
    include/nete/Entity.h
    include/nete/Component.h
    include/nete/nete.h
)

set(TESTS_SOURCES
    tests/nete_tl_tests.cpp
    tests/nete_tests.cpp
)

add_custom_target(nete SOURCES ${SOURCES})

enable_testing()
include_directories(include)
add_executable (nete_tests ${TESTS_SOURCES})
add_test(NAME nete_tests COMMAND nete_tests)
