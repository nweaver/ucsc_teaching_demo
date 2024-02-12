package edu.ucdavis.cs.nweaver


import org.junit.jupiter.api.*

class GraphTest {

    @Test
    fun shortestPath() {

        // The first test is a simple one: A 4 entry graph, all connected by
        // single weight 1 links to the start.
        val graph = makeGraphNodes<String>("A", "B", "C", "D")
        for(item in graph){
            graph.insertEdge("A", item.name, 1.0)
        }
        // The distance to all nodes should be 1, except for A
        for((node, distance, previous) in graph.shortestPath("A")){
            if(node.name == "A"){
                assert(distance == 0.0)
            } else {
                assert(distance == 1.0)
            }
        }
        // And there should be 4 entries returned...
        assert(graph.shortestPath("A").count() == 4)

        // Now we test a disconnected node...
        graph.insertNode("E")
        // We shouldn't have E in it...
        assert(!graph.shortestPath("A").any{it.node.name == "E"})
        assert(graph.shortestPath("A").count() == 4)
        // But we did have 5 nodes
        assert(graph.iterator().asSequence().count() == 5)

        // Now a more complex test:  We have 10 nodes...
        val graph2 = Graph<Int>()
        for(x in 0..<10){
            graph2.insertNode(x)
        }
        // Initially just connected linearly with weight 1 edges.
        for(x in 1..<10){
            graph2.insertEdge(x-1, x, 1.0)
        }

        // Lets make sure the shortest paths are right...
        var i = 0
        for((node, distance, previous) in graph2.shortestPath(0)){
            assert(node.name == i)
            assert(distance == i.toDouble())
            if(i != 0) {
                assert(previous!!.name == i-1)
            }
            i++
        }
        // And make sure we can't get back to 0
        for(i in 0..<10){
            assert(graph2.shortestPath(i).count() == 10-i)
        }

        // Now we fully connect the graph with edges that are JUST
        // big enough we know they can't be in the shortest path...
        for(x in 0 ..<10){
            for(y in 0..<10){
                if(x + 1 != y){
                    graph2.insertEdge(x, y, 11.0)
                } else {
                    // This is a check to make sure that there is proper error
                    // reporting on multiple edges.
                    assertThrows<GraphEdgeError>(){ graph2.insertEdge(x, y, 11.0)}
                }
            }
        }
        for (node in graph2){
            assert(node.outEdges.count() == 10)
        }
        i = 0
        for((node, distance, previous) in graph2.shortestPath(0)){
            assert(node.name == i)
            assert(distance == i.toDouble())
            if(i != 0) {
                assert(previous!!.name == i-1)
            }
            i++
        }
    }
}