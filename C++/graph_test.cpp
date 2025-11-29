#include <gtest/gtest.h>

#include "graph.hpp"
#include <random>

TEST(GraphTest, BasicTest)
{
    const int size = 50;

    auto array = std::vector<int>(size);
    auto array2 = std::vector<int>(size);  
    int i;
    for (i = 0; i < size; ++i) {
        array[i] = i;
        array2[i] = i;
    }
    auto rng = std::default_random_engine {};
    // This test strategy is to create a path from 0-9 in a random order,
    // with each node 1 from the next.  And then an additional set of links
    // of size 11 (guarenteed not to be part of the traversal) are added as well.
    // It then checks that the traversal starting at 0 is correct.
    for (auto k = 0; k < 10; ++k) {
        auto g = std::make_shared<graph<int>>();
        std::shuffle(std::begin(array), std::end(array), rng);
        for(i = 0; i < size; ++i) {
            g->create_node(array[i]);
        }
        std::shuffle(std::begin(array), std::end(array), rng);
        for(i = 0; i < 10; ++i) {
            g->create_link(array[i], (array[i]+1) % size, 1.0);
        }
        i = 0;
        for(auto step : dijkstra_traversal<int>(g, 0)) {
            ASSERT_TRUE(step->current->name == i);
            ASSERT_TRUE(step->distance == float(i));
            if(i != 0) {
                ASSERT_TRUE(step->previous->name == (i-1));
            } else {
                ASSERT_TRUE(step->previous == nullptr);
            }
            i++;
        }
        for(i = 0; i < size; ++i) {
            for(auto j = 0; j < size; ++j) {
                if((i + 1) % size != j) {
                    g->create_link(i,j,size+2);
                }
            }
        }
        i = 0;
        for(auto step : dijkstra_traversal<int>(g, 0)) {
            ASSERT_TRUE(step->current->name == i);
            ASSERT_TRUE(step->distance == float(i));
            i++;
        }
        ASSERT_TRUE(i == size);
        EXPECT_THROW(g->create_link(2, 3, 4), std::domain_error);
        EXPECT_THROW(g->create_link(2, 2, 4), std::domain_error);
    }
}
