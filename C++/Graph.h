//
// Created by Nicholas Weaver on 2/10/24.
//

#ifndef GRAPH_H
#define GRAPH_H
#include <map>
#include <unordered_set>

template <class T> class GraphNode;


template <class T>
class GraphEdge {
    GraphNode<T> *start;
    GraphNode<T> *end;
    double weight;
    GraphEdge(GraphNode<T> *startIn, GraphNode<T> *endIn,
        double weightIn) {
        start = startIn;
        end = endIn;
        weight = weightIn;
    }
};

template <class T>
class GraphNode {
public:
    T name;
    std::unordered_set<GraphEdge<T>> outEdges {};
    std::unordered_set<GraphEdge<T>> inEdges {};
    explicit GraphNode(T nameIn) {
        name = nameIn;
    }

    void connectTo( GraphNode<T>* dest, double weight) {
        for (auto itr: outEdges) {
            if(itr.end == dest) {
                throw std::domain_error("Edge already exists");
            }
        }
        auto edge = new GraphEdge(this, dest, weight);
        this->outEdges.insert(edge);
        dest->inEdges.insert(edge);
    }

    /*
     * Note: This does NOT delete the node but also deletes the
     * out edges and in-edges.  This is needed for the C++ version
     * but not the python or kotlin versions of the application.
     */
    ~GraphNode() {
        for(auto itr : outEdges) {
            auto other = itr.end;
            other->inEdges.erase(itr);
            delete itr;
        }
        for(auto itr : inEdges) {
            auto other = itr.start;
            other->outEdges.erase(itr);
            delete itr;
        }
    }
};




template <class T>
class Graph {
protected:
    std::map<T, std::shared_ptr<GraphNode<T>>> nodes {};
public:

};

/*
 * Fortunately at least for this stuff we can use shared_ptr data
 * and reference counting, since there is no back-pointer from the
 * nodes to these values.
 */
template <class T>
struct DijkstraIterationStep {
    std::shared_ptr<GraphNode<T>> current;
    double distance = HUGE_VAL;
    std::shared_ptr<GraphNode<T>> previous = nullptr;

    explicit DijkstraIterationStep(std::shared_ptr<GraphNode<T>> node) {
        current = node;
    }
};

template <class T>
class DijkstraTraversal {
    DijkstraTraversal(std::shared_ptr<Graph<T>> G, T start) {
        std::unordered_set<std::shared_ptr<DijkstraIterationStep<T>>> workingSet;
        for(auto itr : G->nodes) {
            workingSet.insert(std::make_shared<DijkstraIterationStep<T>>(
                itr));
        }

    }
};


#endif //GRAPH_H
