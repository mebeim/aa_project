#include <utility>
#include <unordered_map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/isomorphism.hpp>
#include <boost/range/algorithm/permutation.hpp>
#include <boost/test/unit_test.hpp>

#include "algos.h"
#include "utils.h"
#include "random_graph.h"

#define REPEAT(n) for (unsigned i__ = 0; i__ < (n); i__++)

typedef boost::property <boost::vertex_name_t, unsigned> VertexProp;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProp> Graph;
typedef VertexDesc<Graph> Vertex;
typedef VertexSizeT<Graph> VertexSz;
typedef std::unordered_map<unsigned, Vertex> VertexMap;

BOOST_AUTO_TEST_SUITE(Fill)

/**
 * Helper function: create a boost graph from a string of GraphViz DOT source
 * and return the graph along with a map name->vertex for each vertex name
 * defined in the DOT source. In this case the "name" of a vertex is an unsigned
 * integer.
 */
template <class Graph>
std::pair<Graph, VertexMap> graph_from_dot(const std::string &source) {
	Graph g;
	boost::dynamic_properties dp;
	VertexMap vm;

	typename boost::property_map<Graph, boost::vertex_name_t>::type name = boost::get(boost::vertex_name, g);
	dp.property("node_id", name);
	boost::read_graphviz(source, g, dp);

	for (auto v : iter_vertices(g))
		vm[v] = boost::get(name, v);

	return {g, vm};
}

/**
 * Ensure that fill() computes the correct chordal completion on a known graph.
 *
 * We also test fill_in() and is_perfect_elimination_order() as they are
 * basically all the same function with different return types.
 */
BOOST_AUTO_TEST_CASE(known_graph) {
	auto [g, v] = graph_from_dot<Graph>("graph { 1--0; 2--0; 3--1; 3--2; 4--0; 4--3; 5--0; 5--1; 5--2; 5--3; }");
	auto order  = {v[4], v[3], v[2], v[1], v[0], v[5]};

	BOOST_REQUIRE(!boost::edge(v[0], v[3], g).second);
	BOOST_REQUIRE(!boost::edge(v[1], v[2], g).second);
	BOOST_CHECK(!is_perfect_elimination_order(g, order));

	auto fillin = fill_in(g, order);
	fill(g, order);

	// The fill-in of this ordered graph should be {0--3, 1--2}
	BOOST_CHECK(boost::edge(v[0], v[3], g).second);
	BOOST_CHECK(boost::edge(v[1], v[2], g).second);
	BOOST_CHECK(fillin.find({v[0], v[3]}) != fillin.end());
	BOOST_CHECK(fillin.find({v[1], v[2]}) != fillin.end());

	// Since the ordered graph is now its own chordal completion, the order
	// should now be a perfect elimination order
	BOOST_CHECK(is_perfect_elimination_order(g, order));
}

/**
 * Ensure that fill() correctly computes an empty fill-in for complete graphs
 * regardless of the elimination order. This checks all elimination orders of a
 * complete graph with 7 vertices.
 *
 * We also test fill_in() and is_perfect_elimination_order() as they are
 * basically all the same function with different return types.
 */
BOOST_AUTO_TEST_CASE(complete_graph_has_empty_fill_in_for_any_order) {
	// edge_prob = 1 => complete graph
	Graph g = gen_random_connected_graph<Graph>(7, 1);
	auto order = gen_random_order(g);

	do {
		auto f = fill_in(g, order);

		BOOST_CHECK_EQUAL(f.size(), 0);
		BOOST_CHECK(is_perfect_elimination_order(g, order));

		Graph g_star;
		boost::copy_graph(g, g_star);
		fill(g_star, order);

		// https://en.wikipedia.org/wiki/Graph_isomorphism
		BOOST_CHECK(boost::isomorphism(g, g_star));
	} while (boost::range::next_permutation(order)); // 7! = 5040
}

/**
 * Ensure that fill_in() correctly computes an empty fill-in for complete graphs
 * and is_perfect_elimination_order() correctly reports true (same as previous
 * test, but larger graphs and not all orders are checked).
 *
 * We do not test fill() here as it modifies the graph and the checking for
 * isomorphism on large graphs is too costly.
 */
BOOST_AUTO_TEST_CASE(complete_graph_has_empty_fill_in) {
	REPEAT(10) {
		// edge_prob = 1 => complete graph
		Graph g = gen_random_connected_graph<Graph>(200, 1);
		auto  o = gen_random_order(g);

		BOOST_CHECK_EQUAL(fill_in(g, o).size(), 0);
		BOOST_CHECK(is_perfect_elimination_order(g, o));
	}
}

/**
 * Ensure that fill() correctly computes an empty fill-in for chordal graphs
 * given a perfect elimination order computed by lex_p().
 *
 * We also test fill_in() and is_perfect_elimination_order() as they are
 * basically all the same function with different return types.
 */
BOOST_AUTO_TEST_CASE(chordal_graph_has_empty_fill_in) {
	REPEAT(10) {
		Graph g = gen_random_chordal_graph<Graph>(200, 10000);
		auto  o = lex_p(g);

		BOOST_CHECK_EQUAL(fill_in(g, o).size(), 0);
		BOOST_CHECK(is_perfect_elimination_order(g, o));
	}
}

BOOST_AUTO_TEST_SUITE_END()
