//
// Created by Nicholas Weaver on 2/10/24.
//

#include "graph.h"


#include <cassert>
#include <random>

// Basically this file is a noop:  Because all our stuff
// is using templates for parameterized typing the result is that
// effectively everythirg is in the header.  However, we have
// our testing code here in the testGraph function

void testGraph() {
    std::cerr << "Initializing graph tests" << std::endl;
    auto array = std::vector<int>(10);
    auto array2 = std::vector<int>(10);

    int i;
    for (i = 0; i < 10; ++i) {
        array[i] = i;
        array2[i] = i;
    }
    auto rng = std::default_random_engine {};
    for (auto k = 0; k < 10; ++k) {
        auto g = std::make_shared<graph<int>>();
        std::shuffle(std::begin(array), std::end(array), rng);
        for(i = 0; i < 10; ++i) {
            g->create_node(array[i]);
        }
        std::shuffle(std::begin(array), std::end(array), rng);
        for(i = 0; i < 10; ++i) {
            g->create_link(array[i], (array[i]+1) % 10, 1.0);
        }
        i = 0;
        for(auto step : dijkstra_traversal<int>(g, 0)) {
            assert(step->current->name == i);
            assert(step->distance == float(i));
            if(i != 0) {
                assert(step->previous->name == (i-1));
            } else {
                assert(step->previous == nullptr);
            }
            i++;
        }
        for(i = 0; i < 10; ++i) {
            for(auto j = 0; j < 10; ++j) {
                if((i + 1) % 10 != j) {
                    g->create_link(i,j,11);
                }
            }
        }
        i = 0;
        for(auto step : dijkstra_traversal<int>(g, 0)) {
            assert(step->current->name == i);
            assert(step->distance == float(i));
            i++;
        }
    }
}