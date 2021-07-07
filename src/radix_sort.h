/**
 * Simple radix-sort implementation for usage in lex_m().
 */

#ifndef RADIX_SORT_H
#define RADIX_SORT_H

#include <vector>
#include <algorithm>
#include <limits>
#include <climits>
#include <boost/type_traits/is_same.hpp>

/**
 * Base-16 LSD radix sort of a collection of unsigned integral values using a
 * map as comparison key.
 *
 * @param begin iterator to the start of the collection to sort
 * @param end   iterator to the end of the collection to sort
 * @param map   map of each value of the collection to the key value used for
 *              comparison
 *
 * @post values between `begin` and `end` are ordered in ascending order
 *       according to `map`
 */
template <class Iterator, class Map>
void radix_sort(Iterator begin, Iterator end, Map &map) {
	typedef typename Iterator::value_type Value;
	typedef typename Map::mapped_type CompareKey;

	static_assert(std::numeric_limits<CompareKey>::is_integer);
	static_assert(!std::numeric_limits<CompareKey>::is_signed);
	static_assert(boost::is_same<typename Map::key_type, Value>::value);

	std::vector<Value> buckets[0x10];
	CompareKey max = 0;
	const unsigned key_bits = sizeof(CompareKey) * CHAR_BIT;

	for (auto it = begin; it != end; it++)
		max = std::max(map[*it], max);

	for (unsigned shift = 0; shift < key_bits && (max >> shift); shift += 4) {
		for (auto it = begin; it != end; it++) {
			Value v = *it;
			buckets[(map[v] >> shift) & 0xf].push_back(v);
		}

		for (size_t i = 0, off = 0; i < 0x10; i++) {
			std::copy(buckets[i].begin(), buckets[i].end(), begin + off);
			off += buckets[i].size();
			buckets[i].clear();
		}
	}
}

#endif // RADIX_SORT_H
