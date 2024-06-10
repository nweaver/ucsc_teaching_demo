#include <iostream>

#include "graph.h"


int main(int argc, char **argv) {
    testGraph();
    std::cout << "Hello, World!" << std::endl;
    for(auto x = 0; x < argc; ++x ){
        std::cout << x << ":" << argv[x] << std::endl;
    }
    return 0;
}
