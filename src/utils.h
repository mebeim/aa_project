/**
 * Common type definitions and other simple commonly-used macros.
 */

#ifndef UTILS_H
#define UTILS_H

#include <unordered_map>
#include <boost/graph/graph_traits.hpp>
#include <boost/range/iterator_range.hpp>

template <class Graph>
using VertexDesc = typename boost::graph_traits<Graph>::vertex_descriptor;

template <class Graph>
using VertexSizeT = typename boost::graph_traits<Graph>::vertices_size_type;

template <class Graph>
using VertexOrder = std::vector<VertexDesc<Graph>>;

template <class Graph>
using EdgeSet = boost::unordered_set<std::pair<VertexDesc<Graph>, VertexDesc<Graph>>>;

#define iter_vertices(g)     boost::make_iterator_range(boost::vertices(g))
#define iter_neighbors(g, v) boost::make_iterator_range(boost::adjacent_vertices(v, g))

#endif // UTILS_H
