#include <iostream>

#include "Graph.h"

int main() {
    auto g = Graph<int>();
    g.createNode(0);
    g.createNode(1);
    g.createLink(0, 1, 1.0);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
