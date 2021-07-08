/**
 * Implementation of the LEX M algorithm described by Rose & Tarjan in
 * "Algorithmic aspects of vertex elimination on graphs".
 *
 * See: https://doi.org/10.1137/0205021
 */

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
 * Compute a minimal elimination order for the given graph.
 *
 * @param  g graph to compute the order for
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
	Label n_unique_labels = 1;
	VertexOrder<Graph> order(n_vertices);
	std::unordered_map<Vertex, Label> label(n_vertices);
	std::unordered_map<Label, VertexSet> reach;
	VertexSet reached;

	// Start with any vertex
	Vertex cur_vertex = *unordered.begin();

	// Number each vertex of the graph in reverse order
	for (size_t index = n_vertices - 1; index < n_vertices; index--) {
		// Assign index to cur_vertex
		unordered.erase(cur_vertex);
		order[index] = cur_vertex;

		reach.clear();
		reached.clear();

		// Mark each neighbor of cur_vertex as reached and increment its label,
		// while also adding it to the queue for reaching other vertices
		for (const auto v : iter_neighbors(g, cur_vertex)) {
			if (unordered.find(v) != unordered.end()) {
				reached.insert(v);
				reach[label[v]++].insert(v);
			}
		}

		// Explore all unnumbered vertices of the graph in order, following
		// the chains of vertices from lowest to highest maximum label
		for (Label l = 0; l < 2 * n_unique_labels; l += 2) {
			// While we have vertices to reach with chains of maximum label up
			// to l
			while (!reach[l].empty()) {
				const auto it = reach[l].begin();
				const auto v  = *it;

				reach[l].erase(it);

				// For all neighbors of the vertex
				for (const auto w : iter_neighbors(g, v)) {
					if (unordered.find(w) != unordered.end() && reached.find(w) == reached.end()) {
						reached.insert(w);

						if (label[w] > l) {
							// We reached this vertex with a chain of lower
							// labeled vertives, increase its label
							reach[label[w]].insert(w);
							label[w]++;
						} else {
							// We reached this vertex, but with a chain of
							// vertices with some higher-or-equal labels, add it
							// to the queue to be explored later
							reach[l].insert(w);
						}
					}
				}
			}
		}

		if (unordered.empty())
			break;


		// Sort and recompute all labels to be [0, 2, 2 * n_unique_labels) while
		// also counting the number of unique labels
		std::vector<Vertex> to_relabel(unordered.begin(), unordered.end());
		radix_sort(to_relabel.begin(), to_relabel.end(), label);

		Label prev_label = label[to_relabel.front()];
		n_unique_labels = 1;

		for (const auto v : to_relabel) {
			if (label[v] != prev_label) {
				n_unique_labels++;
				prev_label = label[v];
			}

			label[v] = 2 * (n_unique_labels - 1);
		}

		// Pick the highest labeled vertex as the next one
		cur_vertex = to_relabel.back();
	}

	return order;
}

#endif // ALGO_LEXM_H
