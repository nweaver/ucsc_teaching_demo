#!/usr/bin/env python3

from collections import deque

class GraphNode():
    def __init__(self, name, data=None):
        self.data = data
        self.name = name
        # We use a set here because we want to efficiently add,
        # remove, and iterate over all edges.
        self.edges = set()

        # We also maintain a set of backward edges from dest->source
        # so we can go backwards even though it is a directed graph.
        self.back_edges = set()
        self.color = "none-set"
        self.previous = None
        
    def connect(self, dest_node, weight=1):
        """Connects this node to another node.  Important notes: It
        assumes a DAG, and you need to connect the other way to be
        sure.  It also does NOT allow multiple connections to the same
        destination, and enforces this with an assertion.
        """
        for e in self.edges:
            if e.dest_node == dest_node:
                raise Exception("An edge already exists")
        edge = GraphEdge(self, dest_node, weight)
        self.edges.add(edge)
        dest_node.back_edges.add(edge)

    def connected(self, dest_node):
        for e in self.edges:
            if e.dest_node == dest_node:
                return True
        return False
        
    def disconnect(self, dest_node):
        edge = None
        for e in self.edges:
            if e.dest_node == dest_node:
                edge = e
        if edge == None:
            raise Exception("No edge exists between {} and {}".format(
                self.name, dest_node.name))
        self.edges.remove(edge)
        dest_node.back_edges.remove(edge)

    def __bool__(self):
        """Needed to keep python from getting too smart because we
        may also have __len__ defined"""
        return True

    def _check_structure(self):
        """A sanity check function that makes sure that every edge is well
        formed"""
        for edge in self.edges:
            assert edge in edge.dest_node.back_edges
            assert edge.source_node == self
        for edge in self.back_edges:
            assert edge in edge.source_node.edges
            assert edge.dest_node == self
        return True
    
    def __repr__(self):
        return ("( {}/{} [".format(self.name, self.data) +
                ", ".join(
                    map(
                        lambda x: "{}->{}".format(x.weight,
                                                  x.dest_node.name), self.edges))
                    + "])")

    def all_edges(self):
        """This iterates over all nodes it is connected TO in
        unspecified order, returning a tuple of destination/weight."""
        if not self.edges:
            return iter([])
        for edge in self.edges:
            yield (edge.dest_node, edge.weight)
        
class GraphEdge():
    def __init__(self, source_node, dest_node, weight=1):
        self.source_node = source_node
        self.dest_node = dest_node
        self.weight = weight

class Graph():
    def __init__(self, dictionary=None):
        self.nodes = {}
        if dictionary:
            for key in dictionary:
                self[key] = dictionary[value]

        
    def __getitem__(self, name):
        """This returns the NODE itself, not the data in the node."""
        if name not in self.nodes:
            raise IndexError("Unable to find {}".format(name))
        return self.nodes[name]

    def __setitem__(self, name, data):
        if name in self.nodes:
            self.nodes[name].data = data
        else:
            self.nodes[name] = GraphNode(name, data)

    def __delitem__(self, name):
        if name not in self.nodes:
            raise IndexError("Unable to find {}".format(name))
        node = self.nodes[name]
        for edge in node.edges:
            edge.dest_node.back_edges.remove(edge)
        for edge in node.back_edges:
            edge.source_node.edges.remove(edge)
        del self.nodes[name]

    def __contains__(self, name):
        return name in self.nodes
        
    def connect(self, source_name, dest_name, weight=1):
        source = self[source_name]
        dest = self[dest_name]
        source.connect(dest, weight)

    def disconnect(self, source_name, dest_name):
        source = self[source_name]
        dest = self[dest_name]
        source.disconnect(dest)
        
    def connected(self, source_name, dest_name):
        source = self[source_name]
        dest = self[dest_name]
        return source.connected(dest)
        
    def check_structure(self):
        """Will raise an assertion failure if it is not well formed"""
        for node in self.nodes:
            self.nodes[node]._check_structure()
        return True


    def dijkstra_traversal(self, node_name):
        start = self.nodes[node_name]
        unvisited = set()
        for n in self.nodes:
            self.nodes[n].distance = float('inf')
            self.nodes[n].previous = None
            unvisited.add(self.nodes[n])
            for (dest, weight) in self.nodes[n].all_edges():
                assert weight > 0
        start.distance = 0
        while len(unvisited) > 0:
            min_node = None
            for node in unvisited:
                if min_node == None:
                    min_node = node
                if node.distance < min_node.distance:
                    min_node = node
            if min_node.distance == float('inf'):
                break
            unvisited.remove(min_node)
            for (dest, weight) in min_node.all_edges():
                if ( dest in unvisited and
                     min_node.distance + weight < dest.distance):
                    dest.distance = min_node.distance + weight
                    dest.previous = min_node
            yield min_node

    def connected_from(self, node_name):
        for item in self.bfs_traversal(node_name):
            pass
        for n in self.nodes:
            if self.nodes[n].color == "white":
                return False
        return True
            
    def bfs_traversal(self, node_name):
        """Does an iterative breadth first search traversal"""
        for n in self.nodes:
            self.nodes[n].color = "white"
            self.nodes[n].previous = None
        start_node = self[node_name]
        queue = deque()
        start_node.color = "gray"
        queue.appendleft(start_node)
        while len(queue) != 0:
            node = queue.pop()
            for (dest, weight) in node.all_edges():
                if dest.color == "white":
                    dest.color = "gray"
                    dest.previous = node
                    queue.appendleft(dest)
            node.color = "black"
            yield node


    def dfs_iterative_traversal(self, node_name):
        """Does an iterative breadth first search traversal"""
        for n in self.nodes:
            self.nodes[n].color = "white"
            self.nodes[n].previous = None
        start_node = self[node_name]
        queue = []
        queue.append(start_node)
        start_node.color = "gray"
        while len(queue) != 0:
            node = queue[len(queue)-1]
            appended = False
            for (dest, weight) in node.all_edges():
                if dest.color == "white":
                    dest.color = "gray"
                    dest.previous = node
                    queue.append(dest)
                    appended = True
            if not appended:
                node = queue.pop()
                node.color = "black"
                yield node

            
    def dfs_traversal(self, node_name):
        def dfs_internal(at):
            at.color = "gray"
            for (dest, edge) in at.all_edges():
                if dest.color == "white":
                    dest.previous = at
                    yield from dfs_internal(dest)
            at.color = "black"
            yield at
        for n in self.nodes:
            self.nodes[n].color = "white"
            self.nodes[n].previous = None
        yield from dfs_internal(self.nodes[node_name])
        
    def __len__(self):
        return len(self.nodes)

    def __repr__(self):
    
        return "{ " + ", ".join(map(lambda x: repr(self.nodes[x]),
                                    self.nodes)) + "}" 

def make_star():
    g = Graph()
    nums = "01234"
    for i in range(5):
        g[i] = "Node{}".format(i)
    for i in range(5):
        g.connect(i, (i+2) % 5)
        g.connect(i, (i-2) % 5)
    return g
        

def samplegraph():
    g = Graph()
    connections = [(0,1,1), (0,5,5),
                   (1,2,20), (1,3,3), (1,4,4),
                   (2,0,2), (3,4,4),
                   (4,2,4)]
    for i in range(6):
        g[i] = "Node{}".format(i)
    for source, dest, weight in connections:
        g.connect(source, dest, weight)
    return g
