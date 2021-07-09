/**
 * Implementation of the FILL algorithm described by Rose & Tarjan in
 * "Algorithmic aspects of vertex elimination on graphs", plus other utility
 * functions using the same algorithm.
 *
 * See: https://doi.org/10.1137/0205021
 */

#ifndef ALGO_FILL_H
#define ALGO_FILL_H

#include <utility>
#include <unordered_set>
#include <unordered_map>

#include "utils.h"

/**
 * Compute the chordal completion of an ordered graph, directly adding new edges
 * to the graph.
 *
 * @param g     graph to compute the chordal completion of
 * @param order ordered sequence of vertices of the graph
 *
 * @pre  `g` is a simple, connected, undirected graph; `order` is an ordered
 *       sequence of the vertices of `g`
 * @post `g` is the chordal completion of the original graph according to
 *       `order`
 */
template <class Graph>
void fill(Graph &g, const VertexOrder<Graph> &order) {
	typedef typename VertexOrder<Graph>::value_type Vertex;
	typedef typename VertexOrder<Graph>::size_type Index;

	BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));

	const auto n_vertices = boost::num_vertices(g);
	std::unordered_map<Vertex, Index> index_of(n_vertices);
	std::unordered_map<Vertex, std::unordered_set<Vertex>> succ(n_vertices);

	for (Index i = 0; i < order.size(); i++)
		index_of[order[i]] = i;

	// Compute initial sets of successors: w is successor of v iff v--w and v
	// comes before w in the given order
	for (const auto v : iter_vertices(g)) {
		for (const auto w : iter_neighbors(g, v)) {
			if (index_of[v] < index_of[w])
				succ[v].insert(w);
		}
	}

	// For each vertex v in the order
	for (auto it = order.begin(); it != order.end() - 1; it++) {
		const Vertex v = *it;
		Index min_index = n_vertices;
		Vertex closest;

		// Find the closest successor of v in the order
		for (const auto w : succ[v]) {
			if (index_of[w] < min_index)
				min_index = index_of[w];
		}

		closest = order[min_index];

		// Mark any other successor of v as a successor of closest and add edges
		// to the graph as necessary (i.e. edges of the deficiency of v)
		for (const auto w : succ[v]) {
			if (w != closest && succ[closest].find(w) == succ[closest].end()) {
				succ[closest].insert(w);
				boost::add_edge(closest, w, g);
			}
		}
	}
}

/**
 * Compute the chordal completion of an ordered graph, and only return edges of
 * the fill-in, without adding them to the graph. This is the same function as
 * fill(), except it does not modify the graph and returns the edges instead.
 *
 * @param g     graph to compute the chordal completion of
 * @param order ordered sequence of vertices of the graph
 * @return edges of the fill-in of the graph as pairs of vertices
 *
 * @pre  `g` is a simple, connected, undirected graph; `order` is an ordered
 *       sequence of the vertices of `g`
 */
template <class Graph>
EdgeSet<Graph> fill_in(const Graph &g, const VertexOrder<Graph> &order) {
	typedef typename VertexOrder<Graph>::value_type Vertex;
	typedef typename VertexOrder<Graph>::size_type Index;

	BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));

	const auto n_vertices = boost::num_vertices(g);
	std::unordered_map<Vertex, Index> index_of(n_vertices);
	std::unordered_map<Vertex, std::unordered_set<Vertex>> succ(n_vertices);
	EdgeSet<Graph> fill_in_edges;

	for (Index i = 0; i < order.size(); i++)
		index_of[order[i]] = i;

	// Compute initial sets of successors: w is successor of v iff v--w and v
	// comes before w in the given order
	for (const auto v : iter_vertices(g)) {
		for (const auto w : iter_neighbors(g, v)) {
			if (index_of[v] < index_of[w])
				succ[v].insert(w);
		}
	}

	// For each vertex v in the order
	for (auto it = order.begin(); it != order.end() - 1; it++) {
		const Vertex v = *it;
		Index min_index = n_vertices;
		Vertex closest;

		// Find the closest successor of v in the order
		for (const auto w : succ[v]) {
			if (index_of[w] < min_index)
				min_index = index_of[w];
		}

		closest = order[min_index];

		// Mark any other successor of v as a successor of closest and add new
		// edges to the fill-in as necessary
		for (const auto w : succ[v]) {
			if (w != closest && succ[closest].find(w) == succ[closest].end()) {
				succ[closest].insert(w);

				if (closest < w)
					fill_in_edges.emplace(closest, w);
				else
					fill_in_edges.emplace(w, closest);
			}
		}
	}

	return fill_in_edges;
}

/**
 * Determine whether the provided order is a perfect elimination order for the
 * given graph. This is the same function as fill(), only that it stops as soon
 * as possible without modifying the graph.
 *
 * @param  g     graph
 * @param  order ordered sequence of vertices of the graph
 * @return true/false whether `order` is a perfect elimination order for `g`
 *
 * @pre `g` is a simple, connected, undirected graph
 */
template <class Graph>
bool is_perfect_elimination_order(const Graph &g, const VertexOrder<Graph> &order) {
	typedef typename VertexOrder<Graph>::value_type Vertex;
	typedef typename VertexOrder<Graph>::size_type Index;

	BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));

	const auto n_vertices = boost::num_vertices(g);
	std::unordered_map<Vertex, Index> index_of(n_vertices);
	std::unordered_map<Vertex, std::unordered_set<Vertex>> succ(n_vertices);

	for (Index i = 0; i < order.size(); i++)
		index_of[order[i]] = i;

	// Compute initial sets of successors: w is successor of v iff v--w and v
	// comes before w in the given order
	for (const auto v : iter_vertices(g)) {
		for (const auto w : iter_neighbors(g, v)) {
			if (index_of[v] < index_of[w])
				succ[v].insert(w);
		}
	}

	// For each vertex v in the order
	for (auto it = order.begin(); it != order.end() - 1; it++) {
		const Vertex v = *it;
		Index min_index = n_vertices;
		Vertex closest;

		// Find the closest successor of v in the order
		for (const auto w : succ[v]) {
			if (index_of[w] < min_index)
				min_index = index_of[w];
		}

		closest = order[min_index];

		// If there is any other successor w of v that is not also a successor
		// of closest then the given order was not a perfect elimination order,
		// as the edge closest--w would be part of the deficiency of v.
		for (const auto w : succ[v]) {
			if (w != closest && succ[closest].find(w) == succ[closest].end())
				return false;
		}
	}

	return true;
}

#endif // ALGO_FILL_H
