#ifndef ALGO_LEXM_H
#define ALGO_LEXM_H

#include <limits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <boost/graph/graph_concepts.hpp>

#include "utils.h"
#include "radix_sort.h"

/**
 * Compute a minimal elimination order of the given graph.
 *
 * @param  g graph to compute the order of
 * @return a minimal elimination order for the graph as an ordered sequence of
 *         all its vertices
 *
 * @pre `g` is a simple, connected, undirected graph
 */
template <class Graph>
VertexOrder<Graph> lex_m(const Graph &g) {
	typedef VertexDesc<Graph> Vertex;
	typedef VertexSizeT<Graph> Label;
	typedef std::unordered_set<Vertex> VertexSet;

	static_assert(!std::numeric_limits<Label>::is_signed);
	BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));

	const auto n_vertices = boost::num_vertices(g);
	VertexSet unordered = std::make_from_tuple<VertexSet>(boost::vertices(g));
	Vertex cur_vertex = *unordered.begin();
	Label n_unique_labels = 1;
	VertexOrder<Graph> order(n_vertices);
	std::unordered_map<Vertex, Label> label(n_vertices);
	std::unordered_map<Label, VertexSet> reach;
	VertexSet reached;

	for (size_t index = n_vertices - 1; index < n_vertices; index--) {
		auto max_label = 2 * (n_unique_labels - 1);

		unordered.erase(cur_vertex);
		order[index] = cur_vertex;

		reach.clear();
		reached.clear();

		for (const auto v : iter_neighbors(g, cur_vertex)) {
			if (unordered.find(v) != unordered.end()) {
				reached.insert(v);
				reach[label[v]++].insert(v);
			}
		}

		for (auto l = max_label; l < max_label; l -= 2) {
			while (!reach[l].empty()) {
				const auto it = reach[l].begin();
				const auto v  = *it;

				reach[l].erase(it);

				for (const auto w : iter_neighbors(g, v)) {
					if (unordered.find(w) != unordered.end() && reached.find(w) == reached.end()) {
						reached.insert(w);

						if (label[w] > l)
							reach[label[w]++].insert(w);
						else
							reach[l].insert(w);
					}
				}
			}
		}

		if (unordered.empty())
			break;

		std::vector<Vertex> to_relabel(unordered.begin(), unordered.end());
		radix_sort(to_relabel.begin(), to_relabel.end(), label);

		Label prev_label = to_relabel.front();
		n_unique_labels = 0;

		for (const auto v : to_relabel) {
			if (label[v] != prev_label) {
				n_unique_labels++;
				prev_label = label[v];
			}

			label[v] = 2 * n_unique_labels;
		}

		cur_vertex = to_relabel.back();
	}

	return order;
}

#endif // ALGO_LEXM_H
