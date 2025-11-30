#include <iostream>

#include "graph.hpp"


int main(int argc, char **argv) {
    std::cout << "Hello, World!" << std::endl;
    for(auto x = 0; x < argc; ++x ){
        std::cout << x << ":" << argv[x] << std::endl;
    }
    return 0;
}