#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>

#include "algos.h"
#include "random_graph.h"

#define REPEAT(n) for (unsigned i__ = 0; i__ < (n); i__++)

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

BOOST_AUTO_TEST_SUITE(LexP)

/**
 * Ensure that the elimination order computed by lex_p() on a chordal graph is
 * perfect (i.e. it has empty fill-in).
 */
BOOST_AUTO_TEST_CASE(order_is_perfect_for_chordal_graphs) {
	REPEAT(10) {
		Graph g = gen_random_chordal_graph<Graph>(500, 75000);
		auto o = lex_p(g);
		auto f = fill_in(g, o);

		BOOST_CHECK_EQUAL(f.size(), 0);
		BOOST_CHECK(is_perfect_elimination_order(g, o));
	}
}

BOOST_AUTO_TEST_SUITE_END()
