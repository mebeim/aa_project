#include <ostream>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/test/unit_test.hpp>

#include "algos.h"
#include "random_graph.h"

#define REPEAT(n) for (unsigned i__ = 0; i__ < (n); i__++)

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;
typedef VertexDesc<Graph> Vertex;
typedef VertexSizeT<Graph> VertexSz;

// Helpers for easy loggin of non-trivial data types
namespace std {
	std::ostream &operator<<(std::ostream &stream, const VertexOrder<Graph> &order) {
		for (const auto v : order)
			stream << v << ' ';
		return stream;
	}

	std::ostream &operator<<(std::ostream &stream, const EdgeSet<Graph> &edges) {
		for (const auto [a, b] : edges)
			stream << '(' << a << ", " << b << ") ";
		return stream;
	}

	std::ostream &operator<<(std::ostream &stream, const Graph &g) {
		boost::write_graphviz(stream, g);
		return stream;
	}
}

BOOST_AUTO_TEST_SUITE(LexM)

/**
 * Helper function: check whether set `a` is strictly contained in set `b`, i.e.
 * `a` has fewer elements than `b` and all elements of `a` are in `b`.
 */
template <class Set>
static bool is_strict_subset(const Set &a, const Set &b) {
	if (a.size() >= b.size())
		return false;

	for (const auto &el : a) {
		if (b.find(el) == b.end())
			return false;
	}

	return true;
}

/**
 * Ensure that the elimination order computed by lex_m() is minimal, that is: no
 * other order exists for the same graph such that its fill-in is strictly
 * contained in the fill-in obtained from the order computed by lex_m(). This is
 * a simple brute force test.
 */
BOOST_AUTO_TEST_CASE(order_is_minimal) {
	REPEAT(50) {
		Graph g = gen_random_connected_graph<Graph>(7, 0.6);

		const auto min_order = lex_m(g);
		const auto min_fill  = fill_in(g, min_order);
		auto cur_order = std::make_from_tuple<std::vector<Vertex>>(boost::vertices(g));

		do {
			if (cur_order == min_order)
				continue;

			auto cur_fill = fill_in(g, cur_order);
			auto ok = !is_strict_subset(cur_fill, min_fill);

			BOOST_CHECK_MESSAGE(ok, "elimination order is non-minimal");

			if (!ok) {
				BOOST_TEST_MESSAGE(g);
				BOOST_TEST_MESSAGE("lex_m() order: " << min_order << "with fill-in: " << min_fill);
				BOOST_TEST_MESSAGE("better order : " << cur_order << "with fill-in: " << cur_fill);
				return;
			}
		} while (boost::range::next_permutation(cur_order)); // 7! = 5040
	}
}

/**
 * Ensure that the elimination order computed by lex_m() on a chordal graph is
 * perfect (i.e. it has empty fill-in). This is true because minimal implies
 * perfect for an elimination order of a chordal graph.
 */
BOOST_AUTO_TEST_CASE(order_is_perfect_for_chordal_graphs) {
	REPEAT(10) {
		Graph g = gen_random_chordal_graph<Graph>(100, 3000);
		auto o = lex_m(g);
		auto f = fill_in(g, o);

		BOOST_CHECK_EQUAL(f.size(), 0);
		BOOST_CHECK(is_perfect_elimination_order(g, o));
	}
}

/**
 * Ensure that the elimination order computed by lex_m() on a complete graph is
 * perfect (i.e. it has empty fill-in). This is true because complete implies
 * chordal.
 */
BOOST_AUTO_TEST_CASE(order_is_perfect_for_complete_graphs) {
	REPEAT(10) {
		// edge_prob = 1 => complete graph
		Graph g = gen_random_connected_graph<Graph>(100, 1);

		auto o = lex_m(g);
		auto f = fill_in(g, o);

		BOOST_CHECK_EQUAL(f.size(), 0);
		BOOST_CHECK(is_perfect_elimination_order(g, o));
	}
}

BOOST_AUTO_TEST_SUITE_END()
