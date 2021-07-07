#ifndef ALGO_FILL_H
#define ALGO_FILL_H

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

	for (const auto v : iter_vertices(g)) {
		for (auto w : iter_neighbors(g, v)) {
			if (index_of[v] < index_of[w])
				succ[v].insert(w);
		}
	}

	for (auto it = order.begin(); it != order.end() - 1; it++) {
		const Vertex v = *it;
		Index min_index = n_vertices;
		Vertex min;

		for (auto w : succ[v]) {
			if (index_of[w] < min_index)
				min_index = index_of[w];
		}

		min = order[min_index];

		for (const auto w : succ[v]) {
			if (w != min && succ[min].find(w) == succ[min].end()) {
				succ[min].insert(w);
				boost::add_edge(min, w, g);
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

	BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));

	const auto n_vertices = boost::num_vertices(g);
	std::unordered_map<Vertex, Index> index_of(n_vertices);
	std::unordered_map<Vertex, std::unordered_set<Vertex>> succ(n_vertices);
	EdgeSet<Graph> fill_in_edges;

	for (Index i = 0; i < order.size(); i++)
		index_of[order[i]] = i;

	for (const auto v : iter_vertices(g)) {
		for (auto w : iter_neighbors(g, v)) {
			if (index_of[v] < index_of[w])
				succ[v].insert(w);
		}
	}

	for (auto it = order.begin(); it != order.end() - 1; it++) {
		const Vertex v = *it;
		Index min_index = n_vertices;
		Vertex min;

		for (auto w : succ[v]) {
			if (index_of[w] < min_index)
				min_index = index_of[w];
		}

		min = order[min_index];

		for (const auto w : succ[v]) {
			if (w != min && succ[min].find(w) == succ[min].end()) {
				succ[min].insert(w);

				if (min < w)
					fill_in_edges.emplace(min, w);
				else
					fill_in_edges.emplace(w, min);
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

	for (const auto v : iter_vertices(g)) {
		for (auto w : iter_neighbors(g, v)) {
			if (index_of[v] < index_of[w])
				succ[v].insert(w);
		}
	}

	for (auto it = order.begin(); it != order.end() - 1; it++) {
		const Vertex v = *it;
		Index min_index = n_vertices;
		Vertex min;

		for (auto w : succ[v]) {
			if (index_of[w] < min_index)
				min_index = index_of[w];
		}

		min = order[min_index];

		for (const auto w : succ[v]) {
			if (w != min && succ[min].find(w) == succ[min].end())
				return false;
		}
	}

	return true;
}

#endif // ALGO_FILL_H
