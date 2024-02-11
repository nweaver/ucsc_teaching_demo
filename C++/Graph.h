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
public:
    std::shared_ptr<GraphNode<T>> start;
    std::shared_ptr<GraphNode<T>> end;
    double weight;
    GraphEdge(std::shared_ptr<GraphNode<T>> startIn,
              std::shared_ptr<GraphNode<T>> endIn,
        double weightIn) {
        start = startIn;
        end = endIn;
        weight = weightIn;
        for(auto itr: start->outEdges) {
            if(itr->end == end) {
                throw std::domain_error("Edge asready exists");
            }
        }
    }
};

template <class T>
class GraphNode {
public:
    T name;
    std::unordered_set<std::shared_ptr<GraphEdge<T>>> outEdges {};
    std::unordered_set<std::shared_ptr<GraphEdge<T>>> inEdges {};
    explicit GraphNode(T nameIn) {
        name = nameIn;
    }

    /*
     * Note: This does NOT delete the node but also deletes the
     * out edges.  This is needed for the C++ version
     * but not the python or kotlin versions of the application.
     *
     * Because graphs contain cycles, the reference counting abilities
     * of std::shared_ptr do not help here, instead we have to manually
     * do this.
     */
    ~GraphNode() {
        for(auto itr : outEdges) {
            auto other = itr->end;
            other->inEdges.erase(itr);
        }
        for(auto itr : inEdges) {
            auto other = itr->start;
            other->outEdges.erase(itr);
        }
        inEdges.clear();
        outEdges.clear();
    }
};




template <class T>
class Graph {
public:
    std::unordered_map<T, std::shared_ptr<GraphNode<T>>> nodes {};
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
        nodes[start]->outEdges.insert(edge);
        nodes[end]->inEdges.insert(edge);
    }
};

/*
 * Fortunately at least for this stuff we can use shared_ptr data
 * and reference counting, since there is no back-pointer from the
 * nodes to these values.
 */
template <class T>
struct DijkstraIterationStep {
public:
    std::shared_ptr<GraphNode<T>> current;
    double distance = HUGE_VAL;
    std::shared_ptr<GraphNode<T>> previous = nullptr;

    explicit DijkstraIterationStep(std::shared_ptr<GraphNode<T>> node) {
        current = node;
    }
};



template <class T>
class DijkstraTraversal {
public:
    std::unordered_map<std::shared_ptr<GraphNode<T>>,
                        std::shared_ptr<DijkstraIterationStep<T>>> workingSet;
    std::shared_ptr<DijkstraIterationStep<T>> currentNode = nullptr;

    DijkstraTraversal(std::shared_ptr<Graph<T>> G, T start) {
        if(!G->nodes.contains(start)) {
            throw std::logic_error("Unable to find the node");
        }
        for(auto itr : G->nodes) {
            auto element = std::make_shared<DijkstraIterationStep<T>>(itr.second);
            if (itr.first == start) {
                element->distance = 0;
                currentNode = element;
            } else {
                workingSet[itr.second] = element;
            }
        }
    }

    DijkstraTraversal* begin() {
        return this;
    }

    DijkstraTraversal* end() {
        return this;
    }


    std::shared_ptr<DijkstraIterationStep<T>> operator*() {
        return currentNode;
    }


    // The ++ operator is the part that does the
    void operator++(){

        currentNode = nullptr;
        if (workingSet.size() == 0) {
            return;
        }
        for (auto itr: workingSet) {
            if(currentNode == nullptr) {
                currentNode = itr.second;
            }
            if(itr.second->distance < currentNode->distance) {
                currentNode = itr;
            }
        }
        workingSet.erase(currentNode->current);
        if(currentNode->distance == HUGE_VAL) {
            currentNode = nullptr;
            return;
        }
        for (auto itr : currentNode->current->outEdges) {
            if(workingSet.contains(itr.end)) {
                auto distance = currentNode->distance + itr->weight;
                if(distance < workingSet[itr->end].distance) {
                    workingSet[itr->end].distance = distance;
                    workingSet[itr->end].previous = currentNode;
                }
            }
        }
    }

    bool operator!=(DijkstraTraversal &other) {
        assert(other == this);
        return currentNode == nullptr || workingSet.size() == 0;
    }
};


#endif //GRAPH_H
