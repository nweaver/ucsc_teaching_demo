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


# And this is code to enable Google Test...
# This is geting gunit so you don't have to...
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()


add_executable(testbinary graph.cpp graph_test.cpp) 
target_link_libraries(
  testbinary
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(testbinary)
