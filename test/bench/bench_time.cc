#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <benchmark/benchmark.h>

#include "algos.h"
#include "random_graph.h"

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

template <unsigned num, unsigned div>
void fill_in_random_graph(benchmark::State& state) {
	const unsigned v = state.range(0);
	auto g = gen_random_connected_graph<Graph>(v, (double)num/div);
	auto o = gen_random_order(g);
	assert(boost::num_vertices(g) == v);

	// Benchmark fill_in() instead of fill() as it is the exact same function as
	// fill(), except that it does not modify the graph, and therefore does not
	// require the overhead of graph creation for every single benchmarking
	// iteration.
	for (auto _ : state)
		benchmark::DoNotOptimize(fill_in(g, o));

	auto n = boost::num_vertices(g) + boost::num_edges(g);
	state.counters["n"] = n;
	state.counters["v"] = boost::num_vertices(g);
	state.SetComplexityN(n);
}

template <unsigned num, unsigned div>
void lex_m_random_graph(benchmark::State& state) {
	const unsigned v = state.range(0);
	auto g = gen_random_connected_graph<Graph>(v, (double)num/div);
	assert(boost::num_vertices(g) == v);

	for (auto _ : state)
		benchmark::DoNotOptimize(lex_m(g));

	const auto n = boost::num_vertices(g) * boost::num_edges(g);
	state.counters["n"] = n;
	state.counters["v"] = boost::num_vertices(g);
	state.SetComplexityN(n);
}

template <unsigned num, unsigned div>
void lex_p_random_graph(benchmark::State& state) {
	const unsigned v = state.range(0);
	auto g = gen_random_connected_graph<Graph>(v, (double)num/div);
	assert(boost::num_vertices(g) == v);

	if (num != div)
		fill(g, lex_m(g));

	for (auto _ : state)
		benchmark::DoNotOptimize(lex_p(g));

	auto n = boost::num_vertices(g) + boost::num_edges(g);
	state.counters["n"] = n;
	state.counters["v"] = boost::num_vertices(g);
	state.SetComplexityN(n);
}

#ifndef NTHREADS
#define NTHREADS 1
#endif

#define bench(func, num, div, start, end, step) \
	BENCHMARK_TEMPLATE(func , num, div)         \
		->DenseRange(start, end, step)          \
		->Complexity(benchmark::oN)             \
		->Unit(benchmark::kMillisecond)         \
		->Threads(NTHREADS)

bench(fill_in_random_graph, 1, 10, 100, 1000, 100); // edge density  10%
bench(fill_in_random_graph, 1,  4, 100, 1000, 100); // edge density  25%
bench(fill_in_random_graph, 1,  2, 100, 1000, 100); // edge density  50%
bench(fill_in_random_graph, 2,  3, 100, 1000, 100); // edge density  66%
bench(fill_in_random_graph, 3,  4, 100, 1000, 100); // edge density  75%
bench(fill_in_random_graph, 1,  1, 100, 1000, 100); // edge density 100% (complete graph)

bench(lex_m_random_graph  , 1, 10, 100, 1000, 100); // edge density  10%
bench(lex_m_random_graph  , 1,  4, 100, 1000, 100); // edge density  25%
bench(lex_m_random_graph  , 1,  2, 100, 1000, 100); // edge density  50%
bench(lex_m_random_graph  , 2,  3, 100, 1000, 100); // edge density  66%
bench(lex_m_random_graph  , 3,  4, 100, 1000, 100); // edge density  75%
bench(lex_m_random_graph  , 1,  1, 100, 1000, 100); // edge density 100% (complete graph)

bench(lex_p_random_graph  , 1, 10, 100, 1000, 100); // edge density  10%
bench(lex_p_random_graph  , 1,  4, 100, 1000, 100); // edge density  25%
bench(lex_p_random_graph  , 1,  2, 100, 1000, 100); // edge density  50%
bench(lex_p_random_graph  , 2,  3, 100, 1000, 100); // edge density  66%
bench(lex_p_random_graph  , 3,  4, 100, 1000, 100); // edge density  75%
bench(lex_p_random_graph  , 1,  1, 100, 1000, 100); // edge density 100% (complete graph)

BENCHMARK_MAIN();
