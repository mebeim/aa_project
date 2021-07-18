/**
 * Random graph and vertex order generation for testing purposes.
 */

#ifndef RANDOM_GRAPH_H
#define RANDOM_GRAPH_H

#include <random>
#include <functional>
#include <vector>
#include <tuple>
#include <deque>
#include <iterator>
#include <unordered_set>
#include <unordered_map>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/connected_components.hpp>

#include "utils.h"

/**
 * Generate a randomic undirected connected graph using an Erdos-Renyi-like
 * model. This is similar to what boost::erdos_renyi_iterator does, except that
 * it is guaranteed to generate a connected graph and it explicitly avoids
 * parallel undirected edges (i.e. it explicitly avoids generating a-b and b-a).
 *
 * @param  n_vertices number of vertices of the generated graph
 * @param  edge_prob  probability (0 <= p <= 1) of having an edge between any
 *                    distinct pair of vertices
 * @return the generated graph
 */
template <class Graph>
Graph gen_random_connected_graph(unsigned n_vertices, double edge_prob) {
	typedef VertexDesc<Graph> Vertex;
	typedef VertexSizeT<Graph> VertexSz;

	static std::mt19937 gen{std::random_device{}()};
	static std::uniform_real_distribution<float> distr_real(0.0, 1.0);
	static auto rand_01 = std::bind(distr_real, gen);

	Graph g;
	std::vector<Vertex> vertices(n_vertices);

	for (unsigned i = 0; i < n_vertices; i++)
		vertices[i] = boost::add_vertex(g);

	// Generate undirected edges with given probability
	for (auto v = vertices.begin() + 1; v != vertices.end(); v++) {
		for (auto w = vertices.begin(); w != v; w++) {
			if (rand_01() <= edge_prob)
				boost::add_edge(*v, *w, g);
		}
	}

	// Find connected components of the generated graph
	std::unordered_map<Vertex, VertexSz> component(boost::num_vertices(g));
	auto map = boost::make_assoc_property_map(component);
	auto n_components = boost::connected_components(g, map);

	if (n_components == 1)
		return g;

	std::unordered_set<VertexSz> seen(n_components);
	auto it = component.begin();
	auto prev_v = it->first;

	seen.insert(it->second);
	component.erase(it);

	// Connect components to make the entire graph connected
	for (auto [v, c] : component) {
		if (seen.find(c) == seen.end()) {
			seen.insert(c);
			boost::add_edge(v, prev_v, g);
			prev_v = v;
		}
	}

	return g;
}

/**
 * Generate a randomic undirected chordal graph as a tree of cliques using the
 * maximal clique generation and expansion method described by Markenzon, Vernet
 * & Araujo in "Two methods for the generation of chordal graphs".
 *
 * See: https://link.springer.com/article/10.1007/s10479-007-0190-4
 *
 * @param  n_vertices number of vertices of the generated graph
 * @param  max_edges  upper bound for the number of edges of the generated graph
 * @return the generated graph
 */
template <class Graph>
Graph gen_random_chordal_graph(unsigned n_vertices, unsigned max_edges) {
	static std::mt19937 gen{std::random_device{}()};
	static std::uniform_int_distribution<unsigned> dist;
	static auto randuint = std::bind(dist, gen);

	std::vector<std::vector<unsigned>> cliques(2);
	std::deque<std::tuple<unsigned, unsigned, unsigned>> L;
	unsigned m = 0;
	unsigned l = 1;

	cliques[1].push_back(1);

	// Expand cliques
	for (unsigned v = 2; v <= n_vertices; v++) {
		const auto i = randuint() % l + 1;
		const auto t = randuint() % cliques[i].size() + 1;

		if (t == cliques[i].size()) {
			cliques[i].push_back(v);
		} else {
			l++;
			cliques.emplace_back();

			cliques[l].clear();
			cliques[l].push_back(v);
			std::copy(cliques[i].begin(), cliques[i].end(), std::back_inserter(cliques[l]));
			L.emplace_back(i, l, t);
		}

		m += t;
	}

	std::unordered_map<unsigned, unsigned> rank;
	std::unordered_map<unsigned, unsigned> parent;
	auto rankp = boost::make_assoc_property_map(rank);
	auto parentp = boost::make_assoc_property_map(parent);

	boost::disjoint_sets<
		boost::associative_property_map<std::unordered_map<unsigned, unsigned>>,
		boost::associative_property_map<std::unordered_map<unsigned, unsigned>>
	> ds(rankp, parentp);

	for (unsigned i = 1; i <= l; i++)
		ds.make_set(i);

	// Merge cliques
	while (!L.empty() && m < max_edges) {
		const auto [a, b, w] = L.back();
		const auto ra = ds.find_set(a);
		const auto rb = ds.find_set(b);
		auto &ca = cliques[ra];
		auto &cb = cliques[rb];
		const auto d = (ca.size() - w) * (cb.size() - w);

		L.pop_back();

		if (m + d <= max_edges) {
			ds.union_set(ra, rb);
			std::copy(cb.begin(), cb.end(), std::back_inserter(ca));
			cb.clear();
			m += d;
		}
	}

	Graph g;
	std::vector<VertexDesc<Graph>> vertices(n_vertices + 1);

	for (unsigned i = 1; i <= n_vertices; i++)
		vertices[i] = boost::add_vertex(g);

	std::set<unsigned> seen;

	// Add all cliques to the graph
	for (const auto &c : cliques) {
		std::vector<unsigned> nnew;
		std::vector<unsigned> old;

		for (const auto v : c) {
			if (seen.find(v) == seen.end()) {
				seen.insert(v);
				nnew.push_back(v);
			} else {
				old.push_back(v);
			}
		}

		// Add all edges of the clique
		for (auto a = nnew.begin(); a != nnew.end(); a++) {
			for (auto b = nnew.begin(); b != a; b++)
				boost::add_edge(vertices[*a], vertices[*b], g);
		}

		// Connect this clique to another clique
		if (old.size() > 0 && nnew.size() > 0 && old[0] != nnew[0]) {
			boost::add_edge(vertices[old[0]], vertices[nnew[0]], g);
		}
	}

	return g;
}

/**
 * Generate a random order for the vertices of a graph.
 *
 * @param  g the graph
 * @return randomly ordered sequence of all the vertices of the graph
 */
template <class Graph>
VertexOrder<Graph> gen_random_order(const Graph &g) {
	auto order = std::make_from_tuple<VertexOrder<Graph>>(boost::vertices(g));
	std::random_shuffle(order.begin(), order.end());
	return order;
}

#endif // RANDOM_GRAPH_H
