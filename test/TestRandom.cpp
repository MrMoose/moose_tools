
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE RandomTests
#include <boost/test/unit_test.hpp>

#include "../Random.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>

using namespace boost::accumulators;

BOOST_AUTO_TEST_CASE(RunOnce) {
	
	BOOST_CHECK_NO_THROW(moose::tools::urand(1));
}


BOOST_AUTO_TEST_CASE(Statistics) {
	
	// well, how can I test randomness.
	// been doing it wrong some times before.	
	// let's try and use boost accumulators
	accumulator_set<unsigned int, stats<tag::mean> > acc;

	// push a random number between 0 and 100 a geat many 
	// times and assume an average of 50.
	// This has proven surprisingly accurate
	for (unsigned int i = 0; i < 100000; ++i) {
		
		BOOST_REQUIRE_NO_THROW(acc(moose::tools::urand(100)));
	}
	
	BOOST_CHECK(mean(acc) < 52);
	BOOST_CHECK(mean(acc) > 48);
}


