//
// Created by Nicholas Weaver on 2/10/24, revised 11/29/25
//

#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <algorithm>
#include <assert.h>

// C++ is somewhat obnoxious here:  You can't do a circular
// reference, so we declare all the classes we will use all up here
// and then declare the bodies later.

// Additionally, since we are doing things with parameterized typing
// on how we name the graph nodes, basically ALL the code needs to
// be in the header, as C++'s parameterization is effectively textual
// substitution: replacing the typename T with whatever type is actually
// used in the code.

// This code also requires C++20.  For some reason, C++'s standard container
// classes (e.g. std::unordered_map and std::unordered_set) 
// did not support a contains() boolean check until C++20!

template <class T>
class graph_node;

template <class T>
class graph_edge;

template <class T>
class graph;

template <class T>
struct dijkstra_iteration_step;

template <class T>
class dijkstra_traversal;

template <class T>
struct dijkstra_traversal_iterator;

// The primary class for a Graph.
//
// This implementation uses an adjacency list within each node, and the graph
// itself has a name->node mapping.
template <class T>
class graph
{
private:

    // A mapping of name to node.  Since we use std::shared_ptr for the nodes,
    // when this graph is deallocated the nodes get their reference counts
    // decremented (and then deallocated)
    std::unordered_map<T, std::shared_ptr<graph_node<T>>> nodes{};

    friend graph_edge<T>;
    friend graph_node<T>;
    friend dijkstra_traversal_iterator<T>;

public:
    void create_node(T name)
    {
        if (nodes.contains(name))
        {
            throw std::domain_error("Node already exists");
        }
        nodes[name] = std::make_shared<graph_node<T>>(name);
    }

    void create_link(T start, T end, double weight)
    {
        if (!nodes.contains(start) || !nodes.contains(end))
        {
            throw std::domain_error("Node does not exist");
        }
        auto edge = std::make_shared<graph_edge<T>>(nodes[start], nodes[end],
                                                    weight);
        nodes[start]->out_edges.insert(edge);
        nodes[end]->in_edges.insert(edge);
    }

    // Note:  This doesn't DELETE the nodes and edges per se:
    // Instead, the shared pointers to the nodes get decremented
    // when the vectors get delected, and those can have their pointers
    // to the edges decremented.
    ~graph()
    {
    }
};

// The class for the edge.  Its pretty simple, with
// just a reference to the starting node, the ending node
// and the weight on the edge.
template <class T>
class graph_edge
{

public:
    const double weight;
    
    // We have this be a weak pointer rather than a shared pointer:
    // This means we don't create a cycle as edges don't create
    // reference counting links to nodes, but nodes DO create
    // refererce counting links to edges.
    const std::weak_ptr<graph_node<T>> start;
    const std::weak_ptr<graph_node<T>> end;

    graph_edge(std::shared_ptr<graph_node<T>> startIn,
               std::shared_ptr<graph_node<T>> endIn,
               double weightIn) : weight(weightIn), start(startIn), end(endIn)
    {

        if (weight <= 0)
        {
            throw std::domain_error("Weights must be positive");
        }
        for (auto itr : start.lock()->out_edges)
        {
            if (itr->end.lock() == end.lock())
            {
                throw std::domain_error("Edge asready exists");
            }
        }
    }
};

// And the class for the node.  This is an adjacency list approach, where
// each node has an unordered set of outward edges and a corresponding unordered
// set of inward edges.  For our traversal we are only using the outEdges, but
// we include both to enable this class to support other graph operations.
template <class T>
class graph_node
{
private:
    std::unordered_set<std::shared_ptr<graph_edge<T>>> out_edges{};
    std::unordered_set<std::shared_ptr<graph_edge<T>>> in_edges{};
    friend dijkstra_traversal_iterator<T>;
    friend graph<T>;
    friend graph_edge<T>;

public:
    const T name;

    explicit graph_node(T nameIn) : name(nameIn)
    {
    }
};

/*
 * This class is used to return step in the iteration:
 * it contains a pointer to the node, the distance to this node from
 * the start, and the prior node on the path (if this isn't the starting
 * node).
 */
template <class T>
struct dijkstra_iteration_step
{
public:
    std::shared_ptr<graph_node<T>> current;
    double distance = HUGE_VAL;
    std::shared_ptr<graph_node<T>> previous = nullptr;

    explicit dijkstra_iteration_step(std::shared_ptr<graph_node<T>> node) : current(node)
    {
    }
};

// This is the heart of the calculation.  In C++ iterators are somewhat
// complex:  the root object needs to support begin() and end() which return
// the iteration objects themselves.  The iteration object for the beginning needs
// to support ++ (increment), * (get the current element) and != (is it at the end).
//
// The end object is effectively a dummy in this case, but it needs to be supported
// anyway.  It will create a needless empty map in the process, but ah well.
//
// Basically, in C++, a loop like

// for(auto a : b) { ... }

// gets converted into:
// auto iterStart = b.begin()
// auto iterEnd = b.end()
// while(iterStart != iterEnd; ) {
//   auto a = *iterStart;
//   ...
//   iterStart++;}

// This means that * will be called for each time through the loop
// and ++ will be called just before the ending is checked.

template <class T>
struct dijkstra_traversal_iterator : std::input_iterator_tag
{
    friend dijkstra_traversal<T>;

private:
    std::unordered_map<std::shared_ptr<graph_node<T>>,
                       std::shared_ptr<dijkstra_iteration_step<T>>>
        working_set;
    std::shared_ptr<dijkstra_iteration_step<T>> current_node = nullptr;
    const std::shared_ptr<graph<T>> working_graph;

    // The private constructor for the iterator.  If its the end it does nothing.
    // If it is the beginning it creates the working set and initializes all the
    // distances to +infinity, except for the start which it initializes to zero.
    //
    // Once done it calls the intnernal iteration function once so that current_node
    // will be pointing to the first node in the traversal (which is the start node).
    // and the first iteration of the calculation will be executed.
    dijkstra_traversal_iterator(std::shared_ptr<graph<T>> graph_ptr, T start, bool is_beginning) : working_graph(graph_ptr)
    {
        if (is_beginning)
        {
            if (!working_graph->nodes.contains(start))
            {
                throw std::logic_error("Unable to find the node");
            }
            for (auto itr : working_graph->nodes)
            {
                auto element = std::make_shared<dijkstra_iteration_step<T>>(itr.second);
                if (itr.first == start)
                {
                    element->distance = 0;
                }
                working_set[itr.second] = element;
            }
            this->iter();
        }
    }

    // And this is the heart of the iteration step.  It clears the current node
    // and first finds the closest node remaining in the working set which is the
    // new current node (or returns if nothing to do).
    //
    // If the distance for the current node is still +infinity it sets the current to
    // nullptr and returns.
    //
    // It removes that node from the working set and then for each outbound edge it looks
    // up the destination.  If that destination is in the working set, it checks the
    // distance.  If the new distance would be less it reduces the distance and updates
    // the previous node on the record.
    void iter()
    {
        current_node = nullptr;
        if (working_set.size() == 0)
        {
            return;
        }
        for (auto itr : working_set)
        {
            if (current_node == nullptr)
            {
                current_node = itr.second;
            }
            if (itr.second->distance < current_node->distance)
            {
                current_node = itr.second;
            }
        }
        working_set.erase(current_node->current);
        if (current_node->distance == HUGE_VAL)
        {
            current_node = nullptr;
            return;
        }
        for (auto itr : current_node->current->out_edges)
        {
            auto end = itr->end.lock();
            assert(end);
            if (working_set.contains(end))
            {
                auto distance = current_node->distance + itr->weight;
                if (distance < working_set[end]->distance)
                {
                    working_set[end]->distance = distance;
                    working_set[end]->previous = current_node->current;
                }
            }
        }
    }

public:
    // The ++ operator is the part that calls the iterator to
    // make sure the current node is available.
    void operator++()
    {
        iter();
    }

    // And the * operator returns the current node.
    std::shared_ptr<dijkstra_iteration_step<T>> operator*()
    {
        return current_node;
    }

    // And this is "is there still data left".  The contract for the
    // iterator says that ++ is called AFTER the data is accessed, so
    // we know it will be executed in the loop in order: If there is
    // no data left the != operation will be checked before the next call
    // to *.
    bool operator!=(dijkstra_traversal_iterator &)
    {
        return current_node != nullptr;
    }
};

//
// And this is the basic shell for the above iterator.
// The constructor accepts the graph and the starting node,
// while the object itself returns the iterator using begin()
// and end().
//
// The reason why C++ requires returing TWO iterators, while
// just about every other language with iteration primatives uses
// one is because C++ bears a lot of old legacy.  Iterators were initially
// designed to do things like iterate over an array's internal storage,
// and the start and end were just pointers to the first element and one plus
// the last element, and the ++ was just doing pointer arithmatic.
template <class T>
class dijkstra_traversal
{

public:
    const std::shared_ptr<graph<T>> working_graph;
    const T start;
    dijkstra_traversal(std::shared_ptr<graph<T>> g, T s) : working_graph(g), start(s)
    {
    }

    dijkstra_traversal_iterator<T> begin()
    {
        return dijkstra_traversal_iterator<T>(working_graph, start, true);
    }

    dijkstra_traversal_iterator<T> begin() const
    {
        return dijkstra_traversal_iterator<T>(working_graph, start, true);
    }

    dijkstra_traversal_iterator<T> end()
    {
        return dijkstra_traversal_iterator<T>(working_graph, start, false);
    }

    dijkstra_traversal_iterator<T> end() const
    {
        return dijkstra_traversal_iterator<T>(working_graph, start, false);
    }
};

#endif // GRAPH_H
