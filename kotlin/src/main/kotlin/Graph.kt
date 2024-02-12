package edu.ucdavis.cs.nweaver

class GraphKeyError(message: String) : Exception(message)
class GraphEdgeError(message: String) : Exception(message)

/*
 * This is an adjacency list representation:  Every node has a list
 * of edges, and every edge has a reference to both the starting node
 * and the ending node for the edge.
 *
 * This is designed to represent a directed graph.  The value of the
 * node is designed to be an arbitrary type.
 */
class GraphNode<T>(val name: T){
    val outEdges: MutableSet<GraphEdge<T>> = mutableSetOf()
    val inEdges: MutableSet<GraphEdge<T>> = mutableSetOf()

    /*
     * The first thing we do with our classes is make sure we
     * have a meaningful toString function, because this greatly
     * eases the debugging process.
     */
    override fun toString(): String {
        return "{Node $name}"
    }
}

/*
 * And the class for edges.  Our edges are directional, from "start" to "end",
 * with a positive weight.
 */
class GraphEdge<T>(
    val start: GraphNode<T>,
    val end: GraphNode<T>,
    val weight: Double = 1.0 ) {
    init {
        if (weight <= 0.0) throw GraphEdgeError("Edge weights must be positive values")
    }

    override fun toString(): String {
        return "$start->$end"
    }
}

/*
 * This data class is returned for the Sequence generator for
 * the shortest path traversal.  The prior node is a nullable
 * because it includes the starting node with distance 0.
 */
data class PathEntry<T>(
    val node: GraphNode<T>,
    var distance: Double,
    var priorNode: GraphNode<T>?) {


    /*
     * We override the data class default toString so we have
     * something representative that makes sense and is good for testing.
     */
    override fun toString(): String {
        return "$node.name/$distance($priorNode?.name)"
    }
}

/*
 * Our graph design is using an adjacency list structure.  Each node
 * has a collection of edges, and each edge knows the start and end.
 *
 * We are assuming a directed graph.  Creating an undirected graph just
 * simply requires inserting one edge for each direction.
 */
class Graph<T>(){
    /*
     * We store the nodes in a key/value pair with the key being the "name"
     * of the node.  This allows us to easily find a particular node as the origin
     * of our shortest-path traversal
     */
    val nodes: MutableMap<T, GraphNode<T>> = mutableMapOf<T, GraphNode<T>>()

    /*
     * Our iterator is over nodes.  This trick is to convert the Map iterator
     * (which does key/value pairs) into an iterator that just has the value portion:
     * We turn the iterator into a Sequence, apply a map to the sequence (returning a
     * new sequence), and get the iterator from that sequence.
     */
    operator fun iterator() = nodes.iterator().asSequence().map{it.value}.iterator()

    /*
     * In inserting new nodes we check to ensure the key is not already in use,
     * but otherwise it accepts an arbitrary node name and creates an associated node.
     */
    fun insertNode(name: T) {
        if(name in nodes){
            throw GraphKeyError("No duplicate keys allowed")
        }
        nodes[name] = GraphNode(name)
    }

    /*
     * For edge creation we insert based on the name (rather than the nodes themselves).
     */
    fun insertEdge(start: T, end: T, weight: Double = 1.0){
        if(start !in nodes || end !in nodes){
            throw GraphKeyError("Failed to find node")
        }

        // We use the !! (non-null assertion) here.  We have checked
        // already that it is valid, but the compiler is unable to
        // assume this because possible concurrent access to a mutable
        // map means keys could vanish between checking and moving
        for(other in nodes[start]!!.outEdges){
            if (other.end == nodes[end]!!){
                throw GraphEdgeError("An edge already exists")
            }
        }
        val edge = GraphEdge(nodes[start]!!, nodes[end]!!, weight)
        nodes[start]!!.outEdges.add(edge)
        nodes[end]!!.inEdges.add(edge)
    }

    /*
     * This implements Dijkstra's shortest path algorithm as a Sequence.
     *
     * Since it is a sequence this means that each node is calculated on
     * demand, allowing someone using this implementation to iterate until
     * they find a destination they intend to reach.
     */
    fun shortestPath(start: T): Sequence<PathEntry<T>> {
        val workingSet = mutableMapOf<GraphNode<T>, PathEntry<T>>()
        if(start !in nodes) {
            throw GraphKeyError("Need to specify a node in the graph")
        }
        // We initialize all nodes to be an infinite distance away
        for ((_, node) in nodes){
            workingSet[node] = PathEntry(node, Double.POSITIVE_INFINITY, null)
        }
        // Except for the start node, that is.
        workingSet[nodes[start]]?.distance = 0.0

        // And here we do the lazy calculation of the shortest path on demand
        return sequence {
            // Now for the main loop, while there are still nodes in the working
            // set...
            while (workingSet.size > 0) {
                // We get the closest node still in the working set.
                var nextEntry: PathEntry<T>? = null
                for ((_, entry) in workingSet) {
                    // If nextEntry is null, the comparison will always be with infinity so
                    // we will take the new entry...
                    nextEntry = if (entry.distance <= (nextEntry?.distance ?: Double.POSITIVE_INFINITY)) {
                        entry
                    } else {
                        nextEntry
                    }
                }
                nextEntry as PathEntry<T>
                workingSet.remove(nextEntry.node)
                // If there are no more reachable nodes we will exit early.
                if (nextEntry.distance == Double.POSITIVE_INFINITY) {
                    return@sequence
                }
                // And now yield the result in the sequence
                yield(nextEntry)

                // This will now trigger if we want the next closest
                // node.  This is an advantage of the Sequence structure:
                // we are able to postpone these calculations until they are
                // actually needed.

                // So now we check each edge in current closest node...
                for (edge in nextEntry.node.outEdges) {
                    val distance = nextEntry.distance + edge.weight
                    // And if the distance is reduced by going from this node,
                    // we update that entry
                    if (edge.end in workingSet &&
                        workingSet[edge.end]!!.distance > distance) {
                        workingSet[edge.end]!!.distance = distance
                        workingSet[edge.end]!!.priorNode = nextEntry.node
                    }
                }
            }
        }
    }
}

fun <T>makeGraphNodes(vararg nodes:T) : Graph<T> {
    val graph = Graph<T>()
    for  (item in nodes){
        graph.insertNode(item)
    }
    return graph
}

