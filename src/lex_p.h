/**
 * Implementation of the LEX P algorithm described by Rose & Tarjan in
 * "Algorithmic aspects of vertex elimination on graphs".
 *
 * See: https://doi.org/10.1137/0205021
 */

#ifndef ALGO_LEXP_H
#define ALGO_LEXP_H

#include <limits>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <boost/graph/graph_concepts.hpp>

#include "utils.h"

/**
 * Compute a perfect elimination order for the given perfect elimination graph.
 *
 * @param  g graph to compute the order for
 * @return a perfect elimination order for the graph as an ordered sequence of
 *         all its vertices
 *
 * @pre `g` is a simple, connected, undirected, perfect elimination graph
 */
template <class Graph>
VertexOrder<Graph> lex_p(const Graph &g) {
	struct Label;
	struct LabeledVertex;

	typedef VertexDesc<Graph> Vertex;
	typedef VertexSizeT<Graph> VertexSz;
	typedef std::unordered_map<Vertex, LabeledVertex *> LabeledVertexMap;

	struct Label {
		LabeledVertexMap vertex_map;
		Label *prev;
		Label *next;

		Label() = delete;
		Label(VertexSz n): vertex_map(n), prev(nullptr), next(nullptr) {}
	};

	struct LabeledVertex {
		Vertex id;
		Label *label;

		LabeledVertex() = delete;
		LabeledVertex(Vertex id, Label *l) : id(id), label(l) {}
	};

	static_assert(!std::numeric_limits<VertexSz>::is_signed);
	BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<Graph>));
	BOOST_CONCEPT_ASSERT((boost::AdjacencyGraphConcept<Graph>));

	const auto n_vertices = boost::num_vertices(g);
	Label *head = new Label(n_vertices);
	LabeledVertexMap unordered(n_vertices);
	VertexOrder<Graph> order(n_vertices);
	std::unordered_map<Label *, Label *> fix;
	LabeledVertex *cur_vertex;

	// Assign the empty label to all the vertices of the graph
	for (const auto id : iter_vertices(g)) {
		LabeledVertex *v = new LabeledVertex(id, head);
		head->vertex_map[id] = v;
		unordered[id] = v;
	}

	// Number each vertex of the graph in reverse order
	for (auto index = n_vertices - 1; index < n_vertices; index--) {
		cur_vertex = nullptr;

		// Find cur_vertex as the highest-labeled unnumbered vertex scanning the
		// linked list of labels
		for (auto label = head; !cur_vertex && label; label = label->next) {
			for (const auto [id, v] : label->vertex_map) {
				auto it = unordered.find(id);

				if (it != unordered.end()) {
					cur_vertex = v;
					unordered.erase(it);
					break;
				}
			}
		}

		// Assign index to cur_vertex
		assert(cur_vertex);
		order[index] = cur_vertex->id;

		// For each unnumbered neighbor of the current vertex
		for (const auto neighbor_id : iter_neighbors(g, cur_vertex->id)) {
			auto it = unordered.find(neighbor_id);

			if (it != unordered.end()) {
				// Create a new label (if not already created) which preceeds
				// the current neighbor's label of exactly one position in the
				// list of labels
				auto neighbor = (*it).second;
				auto prev_it = fix.find(neighbor->label);
				Label *new_label;

				if (prev_it != fix.end()) {
					new_label = (*prev_it).second;
				} else {
					new_label = new Label(1);
					fix[neighbor->label] = new_label;
				}

				// Remove this neighbor from its current label and assign it to
				// the newly created label
				neighbor->label->vertex_map.erase(neighbor_id);
				neighbor->label = new_label;
				neighbor->label->vertex_map[neighbor_id] = neighbor;
			}
		}

		// Add newly created labels to the list
		for (const auto [label, new_label] : fix) {
			if (label->prev)
				label->prev->next = new_label;
			else
				head = new_label;

			new_label->next = label;
			new_label->prev = label->prev;
			label->prev = new_label;
		}

		fix.clear();
	}

	while (head) {
		auto nxt = head->next;

		for (auto v : head->vertex_map)
			delete v.second;
		delete head;

		head = nxt;
	}

	return order;
}

#endif // ALGO_LEXP_H
