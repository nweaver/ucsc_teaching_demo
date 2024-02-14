//
// Created by Nicholas Weaver on 2/10/24.
//

#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <map>
#include <unordered_set>

// C++ is somewhat obnoxious here:  You can't do a circular
// reference, so we declare all the classes we will use all up here
// and then declare the bodies later.

// Additionally, since we are doing things with parameterized typing
// on how we name the graph nodes, basically ALL the code needs to
// be in the header, as C++'s parameterization is effectively textual
// substitution: replacing the typename T with whatever type is actually
// used in the code.

// This code also requires C++20.  For some reason, C++'s standard container
// classes (e.g. std::unordered_map) did not support a contains() boolean check
// until C++20!

template <class T> class GraphNode;
template <class T> class GraphEdge;
template <class T> class Graph;
template <class T> struct DijkstraIterationStep;
template <class T> class DijkstraTraversal;
template <class T> struct DijkstraTraversalIterator;

// The primary class for a Graph.
//
// This implementation uses an adjacency list within each node, and the graph
// itself has a name->node mapping.
template <class T>
class Graph {
private:
    std::unordered_map<T, std::shared_ptr<GraphNode<T>>> nodes {};
    friend GraphEdge<T>;
    friend GraphNode<T>;
    friend DijkstraTraversalIterator<T>;

public:
    void createNode(T name) {
        if(nodes.contains(name)) {
            throw std::domain_error("Node already exists");
        }
        nodes[name] = std::make_shared<GraphNode<T>>(name);
    }

    void createLink(T start, T end, double weight) {
        if(!nodes.contains(start) || !nodes.contains(end)) {
            throw std::domain_error("Node does not exist");
        }
        auto edge = std::make_shared<GraphEdge<T>>(nodes[start], nodes[end],
            weight);
        nodes[start]->out_edges.insert(edge);
        nodes[end]->in_edges.insert(edge);
    }

    // Note:  This doesn't DELETE the nodes and edges per se:
    // Instead, it removes the LINKS between all the nodes and edges.
    //
    // C++ "smart" std::shared_ptr are not actually all that smart.  They
    // are simply reference counts.  But because a graph has cycles, a
    // reference counting collector can't actually remove the graph.
    //
    // So when the graph is deleted, we prune the internal connections
    // so that the nodes CAN be cleaned up by the reference counter safely.
    //
    // This is especially important since the iterator itself returns
    // structures with shared_ptr pointing to the nodes, so the node's lifetime
    // can excee the lifetime of the enclosing graph.  IF we didn't use "smart"
    // pointers for all the nodes/edges we could end up with use after
    // free errors.
    ~Graph() {
        for (auto nodePair: nodes) {
            auto node = nodePair.second;
            for (auto itr: node->out_edges) {
                auto other = itr->end;
                other->in_edges.erase(itr);
            }
            for (auto itr: node->in_edges) {
                auto other = itr->start;
                other->out_edges.erase(itr);
            }
            node->in_edges.clear();
            node->out_edges.clear();
        }
    }
};



// The class for the edge.  Its pretty simple, with
// just a reference to the starting node, the ending node
// and the weight on the edge.
template <class T>
class GraphEdge {

public:
    const double weight;
    const std::shared_ptr<GraphNode<T>> start;
    const std::shared_ptr<GraphNode<T>> end;

    GraphEdge(std::shared_ptr<GraphNode<T>> startIn,
              std::shared_ptr<GraphNode<T>> endIn,
        double weightIn) : weight(weightIn), start(startIn), end(endIn) {

        if(weight <= 0) {
            throw std::domain_error("Weights must be positive");
        }
        for(auto itr: start->out_edges) {
            if(itr->end == end) {
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
class GraphNode {
private:
    std::unordered_set<std::shared_ptr<GraphEdge<T>>> out_edges {};
    std::unordered_set<std::shared_ptr<GraphEdge<T>>> in_edges {};
    friend DijkstraTraversalIterator<T>;
    friend Graph<T>;
    friend GraphEdge<T>;

public:
    const T name;

    explicit GraphNode(T nameIn) : name(nameIn){
    }

};


/*
 * This class is used to return step in the iteration:
 * it contains a pointer to the node, the distance to this node from
 * the start, and the prior node on the path (if this isn't the starting
 * node).
 */
template <class T>
struct DijkstraIterationStep {
public:
    std::shared_ptr<GraphNode<T>> current;
    double distance = HUGE_VAL;
    std::shared_ptr<GraphNode<T>> previous = nullptr;

    explicit DijkstraIterationStep(std::shared_ptr<GraphNode<T>> node) : current(node) {
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
struct DijkstraTraversalIterator: std::input_iterator_tag {
    friend DijkstraTraversal<T>;
private:
    std::unordered_map<std::shared_ptr<GraphNode<T>>,
                    std::shared_ptr<DijkstraIterationStep<T>>> working_set;
    std::shared_ptr<DijkstraIterationStep<T>> current_node = nullptr;
    const std::shared_ptr<Graph<T>> graph;

    // The private constructor for the iterator.  If its the end it does nothing.
    // If it is the beginning it creates the working set and initializes all the
    // distances to +infinity, except for the start which it initializes to zero.
    //
    // Once done it calls the intnernal iteration function once so that current_node
    // will be pointing to the first node in the traversal (which is the start node).
    // and the first iteration of the calculation will be executed.
    DijkstraTraversalIterator(std::shared_ptr<Graph<T>> graph_ptr, T start, bool is_beginning) :
    graph(graph_ptr) {
        if(is_beginning) {
            if(!graph->nodes.contains(start)) {
                throw std::logic_error("Unable to find the node");
            }
            for(auto itr : graph->nodes) {
                auto element = std::make_shared<DijkstraIterationStep<T>>(itr.second);
                if (itr.first == start) {
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
    void iter() {
        current_node = nullptr;
        if (working_set.size() == 0) {
            return;
        }
        for (auto itr: working_set) {
            if(current_node == nullptr) {
                current_node = itr.second;
            }
            if(itr.second->distance < current_node->distance) {
                current_node = itr.second;
            }
        }
        working_set.erase(current_node->current);
        if(current_node->distance == HUGE_VAL) {
            current_node = nullptr;
            return;
        }
        for (auto itr : current_node->current->out_edges) {
            if(working_set.contains(itr->end)) {
                auto distance = current_node->distance + itr->weight;
                if(distance < working_set[itr->end]->distance) {
                    working_set[itr->end]->distance = distance;
                    working_set[itr->end]->previous = current_node->current;
                }
            }
        }
    }

public:
    // The ++ operator is the part that calls the iterator to
    // make sure the current node is available.
    void operator++() {
        iter();
    }

    // And the * operator returns the current node.
    std::shared_ptr<DijkstraIterationStep<T>> operator*() {
        return current_node;
    }

    // And this is "is there still data left".  The contract for the
    // iterator says that ++ is called AFTER the data is accessed, so
    // we know it will be executed in the loop in order: If there is
    // no data left the != operation will be checked before the next call
    // to *.
    bool operator!=(DijkstraTraversalIterator &) {
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
class DijkstraTraversal {

public:
    const std::shared_ptr<Graph<T>> graph;
    const T start;
    DijkstraTraversal(std::shared_ptr<Graph<T>> g, T s) : graph(g), start(s){
    }

    DijkstraTraversalIterator<T> begin() {
        return DijkstraTraversalIterator<T>(graph, start, true);
    }

    DijkstraTraversalIterator<T> begin() const {
        return DijkstraTraversalIterator<T>(graph, start, true);
    }

    DijkstraTraversalIterator<T> end() {
        return DijkstraTraversalIterator<T>(graph, start, false);
    }

    DijkstraTraversalIterator<T> end() const {
        return DijkstraTraversalIterator<T>(graph, start, false);
    }
};

void testGraph();


#endif //GRAPH_H
