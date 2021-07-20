#include <vector>
#include <random>
#include <functional>
#include <climits>
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include "radix_sort.h"

BOOST_AUTO_TEST_SUITE(RadixSort)

typedef boost::mpl::list<
	unsigned char,
	unsigned short,
	unsigned,
	unsigned long
> value_types;

/**
 * Ensure that radix_sort() correctly sorts elements.
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(radix_sort_works, KeyValueT, value_types) {
	static std::mt19937 gen{std::random_device{}()};
	static std::uniform_int_distribution<unsigned> value_dist;
	static std::uniform_int_distribution<KeyValueT> key_dist;
	static auto randval = std::bind(value_dist, gen);
	static auto randkey = std::bind(key_dist, gen);

	std::vector<unsigned> v(1000);
	std::unordered_map<unsigned, KeyValueT> map(1000);

	for (auto it = v.begin(); it != v.end(); it++) {
		*it = randval();
		map[*it] = randkey();
	}

	radix_sort(v.begin(), v.end(), map);

	for (auto it = v.begin(); it != v.end() - 1; it++)
		BOOST_CHECK_LE(map[*it], map[*(it + 1)]);
}

BOOST_AUTO_TEST_SUITE_END()
