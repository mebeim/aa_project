#include <cstdio>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>

#include "algos.h"
#include "random_graph.h"
#include "bench_mem_trace.h"

#define do_not_optimize(v) asm volatile("" : : "r,m"(v) : "memory");

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

const unsigned vertices[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

template <unsigned num, unsigned div>
void fill_random_graph(void) {
	char name[128];

	for (const auto v : vertices) {
		Graph g = gen_random_connected_graph<Graph>(v, (double)num/div);
		auto o = gen_random_order(g);
		const unsigned n = boost::num_vertices(g) + boost::num_edges(g);

		sprintf(name, "fill_random_graph<%u,%u> v=%u n=%u", num, div, v, n);
		start_trace(name);
		fill(g, o);
		stop_trace();
	}
}

template <unsigned num, unsigned div>
void lex_m_random_graph(void) {
	char name[128];

	for (const auto v : vertices) {
		Graph g = gen_random_connected_graph<Graph>(v, (double)num/div);
		const unsigned n = boost::num_vertices(g) * boost::num_edges(g);

		sprintf(name, "lex_m_random_graph<%u,%u> v=%u n=%u", num, div, v, n);
		start_trace(name);
		do_not_optimize(lex_m(g));
		stop_trace();
	}
}

template <unsigned num, unsigned div>
void lex_p_random_graph(void) {
	char name[128];

	for (const auto v : vertices) {
		Graph g = gen_random_connected_graph<Graph>(v, (double)num/div);
		const unsigned n = boost::num_vertices(g) + boost::num_edges(g);

		if (num != div)
			fill(g, lex_m(g));

		sprintf(name, "lex_p_random_graph<%u,%u> v=%u n=%u", num, div, v, n);
		start_trace(name);
		do_not_optimize(lex_p(g));
		stop_trace();
	}
}

int main(void) {
	fill_random_graph <1, 10>(); // edge density  10%
	fill_random_graph <1,  4>(); // edge density  25%
	fill_random_graph <1,  2>(); // edge density  50%
	fill_random_graph <2,  3>(); // edge density  66%
	fill_random_graph <3,  4>(); // edge density  75%
	fill_random_graph <1,  1>(); // edge density 100% (complete graph)

	lex_m_random_graph<1, 10>(); // edge density  10%
	lex_m_random_graph<1,  4>(); // edge density  25%
	lex_m_random_graph<1,  2>(); // edge density  50%
	lex_m_random_graph<2,  3>(); // edge density  66%
	lex_m_random_graph<3,  4>(); // edge density  75%
	lex_m_random_graph<1,  1>(); // edge density 100% (complete graph)

	lex_p_random_graph<1, 10>(); // edge density  10%
	lex_p_random_graph<1,  4>(); // edge density  25%
	lex_p_random_graph<1,  2>(); // edge density  50%
	lex_p_random_graph<2,  3>(); // edge density  66%
	lex_p_random_graph<3,  4>(); // edge density  75%
	lex_p_random_graph<1,  1>(); // edge density 100% (complete graph)

	return 0;
}
