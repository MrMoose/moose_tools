
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE StringTests
#include <boost/test/unit_test.hpp>

#include "../String.hpp"

#include <set>

using namespace moose::tools;

BOOST_AUTO_TEST_CASE(TruncateSimple) {
	
	std::string s("Moose was here");

	BOOST_REQUIRE_NO_THROW(truncate(s, 5));
	BOOST_CHECK(s == "Moose");
	BOOST_REQUIRE_NO_THROW(truncate(s, 15));
	BOOST_CHECK(s == "Moose");
	BOOST_REQUIRE_NO_THROW(truncate(s, 1));
	BOOST_CHECK(s == "M");
	BOOST_REQUIRE_NO_THROW(truncate(s, 0));
	BOOST_CHECK(s.empty());
}


