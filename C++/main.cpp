#include <iostream>

#include "Graph.h"

int main() {
    auto g = std::make_shared<Graph<int>>();
    g->createNode(0);
    g->createNode(1);
    g->createLink(0, 1, 1.0);
    std::cout << "Starting function" << std::endl;
    for(auto step : DijkstraTraversal<int>(g, 0)) {
        std::cout << "Node traversed" << std::endl;
    }
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
