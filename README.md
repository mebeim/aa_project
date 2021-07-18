Algorithmic aspects of vertex elimination on graphs
===================================================

This repo contains the optional project for the Advanced Algorithms part of the
Computer Science and Engineering Master's Degree course
[*Advanced Algorithms and Parallel Programming* (095946 - A.Y. 2020/21)][course]
of Politecnico di Milano. The goal of the project is to create, test and
benchmark a C++ implementation of the algorithms described in
[*Algorithmic aspects of vertex elimination on graphs*][paper] by Rose, Lueker &
Tarjan.

I've implemented the algorithms as a generic header-only C++ template library
(following the C++17 standard) to be used alongside the powerful
[Boost Graph Library][lib-boost-graph], supporting all the different Graph
implementations provided by it.

Implemented algorithms
----------------------

The authors of this paper propose three algorithms for efficient computation of
vertex elimination orderings on *simple, connected, undirected* (SCU for
brevity) graphs:

- FILL: an *O(V+E)* algorithm to compute the
  [chordal completion][wiki-chordal-completion] of any SCU graph.
- LEX M: an *O(VE)* algorithm to compute a minimal vertex elimination order of
  any SCU graph.
- LEX P: an *O(V+E)* algorithm to compute a perfect vertex elimination order for
  a [chordal graph][wiki-chordal-graph]. This algorithm is also known as
  [lexicographic breadth-first search][wiki-lex-p].

### Errors in the paper

I've discovered the following errors in the algorithms described in the paper:

1. The LEX M algorithm presents some logical/typographical errors in the
   "search" loop:

   - All the `k`s that appear inside the loop should be `j`s.
   - The loop should run in the opposite direction with `j` from `1` to `k`: the
     graph needs to be explored starting from lower labeled vertices.
   - `if l(z) < k` should be `if l(z) > j`: the label of vertex `z` should only
     be increased if we can reach it through a chain of *lower-labeled*
     vertices.

2. The BFS labeling algorithm (outside the scope of this project) also presents
   one logical error: in the "explore" loop, new vertices should only be added
   to the queue if not already present. Adding vertices twice will lead to wrong
   labeling.

Building
--------

This is a header-only template library and thus *does not need any building*.
The only dependency when using this library is [Boost Graph][lib-boost-graph]
(development headers and dynamic library). Compile with (at least) `-std=c++17`
and link with `-lboost_graph`.

Building unit tests requires [Boost Graph][lib-boost-graph] and
[Boost Test][lib-boost-test] developement headers and dynamic libraries,
packaged on any major Linux distro (e.g. `libboost-dev` on Debian/Ubuntu which
provides the complete Boost library).

Building benchmarks requires the [Google Benchmark][lib-benchmark] development
headers and static library. This is expected to be manually cloned and built in
the `benchmark/` directory (in the same directory of the `Makefile`).

Build using `make`, output binaries in the `build/` directory:

```bash
make                # build test and benchmark binaries
make run_tests      # build and run tests only
make run_benchmark  # build and run benchmarks only
```

Testing
-------

TODO!

Benchmarks
----------

TODO!

---

*Copyright &copy; 2021 Marco Bonelli. Licensed under the Apache License 2.0.*

[course]:                  https://www4.ceda.polimi.it/manifesti/manifesti/controller/ManifestoPublic.do?EVN_DETTAGLIO_RIGA_MANIFESTO=evento&aa=2020&k_cf=225&k_corso_la=481&k_indir=T2A&codDescr=095946&lang=IT&semestre=2&idGruppo=4152&idRiga=253856
[paper]:                   https://epubs.siam.org/doi/10.1137/0205021
[lib-boost-graph]:         https://www.boost.org/doc/libs/1_76_0/libs/graph/doc/index.html
[lib-boost-test]:          https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html
[lib-benchmark]:           https://github.com/google/benchmark
[wiki-chordal-graph]:      https://en.wikipedia.org/wiki/Chordal_graph
[wiki-chordal-completion]: https://en.wikipedia.org/wiki/Chordal_completion
[wiki-lex-p]:              https://en.wikipedia.org/wiki/Lexicographic_breadth-first_search