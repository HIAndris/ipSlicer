cmake_minimum_required(VERSION 4.0)
project(ipSlicer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ipSlicer src/main.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ ${CMAKE_EXE_LINKER_FLAGS}")
endif()