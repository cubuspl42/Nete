cmake_minimum_required(VERSION 3.1)

project(Nete)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

set(SOURCES
    include/nete.h
    include/Entity.h
    include/Component.h
)
add_custom_target(nete SOURCES ${SOURCES})

enable_testing()
add_executable (nete_tests tests/nete_tests.cpp)
add_test(NAME nete_tests COMMAND nete_tests)
