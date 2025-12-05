#!/usr/bin/env python3
# Do not remove the above line, it is needed for testing

import sys

from graph import Graph, make_star, samplegraph
import random
import unittest

testdict = {"a": 1, "b": 2, "c": 3}

class TestGraphMethods(unittest.TestCase):
    def test_creation(self):
        g = Graph()

    def test_insertion(self):
        g = Graph()
        for key in testdict:
            g[key] = testdict[key]
        self.assertFalse(g.connected("a", "b"))
        g.connect("a", "b")
        self.assertTrue(g.connected("a", "b"))
        g.disconnect("a", "b")
        self.assertFalse(g.connected("a", "b"))
        
    def test_big_graph(self):
        g = Graph()
        side1 = []
        side2 = []
        for x in range(100):
            side1.append("a{}".format(x))
            side2.append("b{}".format(x))
        random.shuffle(side1)
        random.shuffle(side2)
        for e in side1:
            g[e] = "a-node"
        for e in side2:
            g[e] = "b-node"
        random.shuffle(side1)
        random.shuffle(side2)
        for e in side1:
            for e2 in side2:
                g.connect(e, e2)

        random.shuffle(side1)
        random.shuffle(side2)
        for e in side1:
            for e2 in side1:
                self.assertFalse(g.connected(e, e2))
            for e2 in side2:
                self.assertTrue(g.connected(e, e2))

        random.shuffle(side1)
        random.shuffle(side2)
        for e in side2:
            for e2 in side1:
                self.assertFalse(g.connected(e, e2))
            for e2 in side2:
                self.assertFalse(g.connected(e, e2))

        random.shuffle(side1)
        random.shuffle(side2)
        for e in side2:
            for e2 in side2:
                self.assertFalse(g.connected(e, e2))
                g.connect(e, e2)
                self.assertTrue(g.connected(e, e2))

        random.shuffle(side1)
        random.shuffle(side2)
        length = len(g)
        deleted = set()
        for delnode in side2:
            del g[delnode]
            deleted.add(delnode)
            g.check_structure()
            length += -1
            self.assertEqual(length, len(g))
            for e in side2:
                for e2 in side2:
                    if e not in deleted and e2 not in deleted:
                        self.assertTrue(g.connected(e, e2))

    def test_bft(self):
        star = make_star()
        for item in star.bfs_traversal(0):
            print(item.data)

    def test_dft(self):
        star = make_star()
        for item in star.dfs_traversal(0):
            print(item.data)

    def test_dijkstra(self):
        star = make_star()
        for item in star.dijkstra_traversal(0):
            print("{}: {}, <-{}".format(item.data,
                                        item.distance,
                                        item.previous))
        g = Graph()
        for i in range(0, 10):
            g[i] = i
        for i in range(0, 10):
            g.connect(i,(i+1)%10, 1)
        g[-1] = -1
        g.connect(-1, 0, 1)
        i = 0;
        for item in g.dijkstra_traversal(0):
            self.assertTrue(item.name == i)
            self.assertTrue(item.distance == i)
            i = i + 1
        self.assertTrue(i == 10)
        for i in range(0, 10):
            for j in range(0, 10):
                if (i + 1) % 10 != j:
                    g.connect(i, j, 11)
        i = 0;
        for item in g.dijkstra_traversal(0):
            self.assertTrue(item.name == i)
            self.assertTrue(item.distance == i)
            i = i + 1
        self.assertTrue(i == 10)

        g[10] = 10

        i = 0;
        for item in g.dijkstra_traversal(0):
            self.assertTrue(item.name == i)
            self.assertTrue(item.distance == i)
            i = i + 1
        self.assertTrue(i == 10)

        i = 0;
        for item in g.dijkstra_traversal(10):
            i = i + 1
        self.assertTrue(i == 1)
        

    def test_print_bfs(self):
        g = samplegraph()
        print("BFS of sample graph", g)
        for item in g.bfs_traversal(0):
            print(item)
        
    def test_connected_from(self):
        g = samplegraph()
        print("Connected test")
        self.assertTrue(g.connected_from(0))
        self.assertFalse(g.connected_from(5))

    def test_sample_shortest(self):
        g = samplegraph()
        for item in g.dijkstra_traversal(0):
            if item.previous != None:
                print("{}/{}/{}".format(item.name, item.distance, item.previous.name))
        
"""Run the unit tests"""
if __name__ == '__main__':
    try:
        unittest.main()
    except SystemExit:
        pass
