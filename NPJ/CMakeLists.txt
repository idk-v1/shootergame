cmake_minimum_required(VERSION 4.2)
project(NPJLib VERSION 1.0.0 LANGUAGES CXX)

add_library(npjlib INTERFACE)
add_library(npjlib::npjlib ALIAS npjlib)

target_include_directories(npjlib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(npjlib INTERFACE cxx_std_23)
set(CMAKE_CXX_STANDARD 26)

include_directories(.)
include_directories(include/NPJ/intrepid_internals)

add_executable(NPJLib
        include/NPJ/intrepid_internals/VectorInternal.h
        include/NPJ/BaseInjection.h
        include/NPJ/Concepts.h
        include/NPJ/Macros.h
        include/NPJ/Vector.h
        main.cpp)
