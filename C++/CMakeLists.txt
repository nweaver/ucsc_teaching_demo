cmake_minimum_required(VERSION 3.28)
project(C__)


set (CMAKE_CXX_STANDARD 20)

# This is "Turn on a lot the hardening stuff.  There is a 
# minor performance impact but hey, better Safe than Sorry"
# Feel free to turn OFF the optimizer for better debugging however.
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g \
-O2 \
-Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-D_GLIBCXX_ASSERTIONS \
-fopenmp")

set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra --coverage -D_GLIBCXX_ASSERTIONS")


add_executable(hello main.cpp
        graph.cpp
        graph.hpp)
