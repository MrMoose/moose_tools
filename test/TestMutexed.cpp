
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE MutexedTests
#include <boost/test/unit_test.hpp>

#include "../Mutexed.hpp"
#include "../Error.hpp"

using namespace moose::tools;

class MutexedClass : public Mutexed< MutexedClass > {

	public:
		MutexedClass() = default;
		MutexedClass(const MutexedClass &n_other) = delete;
		~MutexedClass() = default;
};

BOOST_AUTO_TEST_CASE(compile_read_lock) {

	BOOST_TEST_MESSAGE("using mutexed class read lock");

	MutexedClass c;
	boost::uint64_t inc1;
	boost::uint64_t inc2;


	BOOST_CHECK_NO_THROW(inc1 = c.incarnation());
	
	{
		MutexedClass::read_lock rl = c.acquire_read_lock();
	}
	BOOST_CHECK_NO_THROW(inc2 = c.incarnation());
	BOOST_CHECK(inc1 == inc2);
}

BOOST_AUTO_TEST_CASE(compile_write_lock) {

	BOOST_TEST_MESSAGE("using mutexed class write lock");

	MutexedClass c;
	boost::uint64_t inc1;
	boost::uint64_t inc2;

	BOOST_CHECK_NO_THROW(inc1 = c.incarnation());
	
	{
		MutexedClass::write_lock wl = c.acquire_write_lock();
	}

	BOOST_CHECK_NO_THROW(inc2 = c.incarnation());
	BOOST_CHECK(inc2 == (inc1 + 1));
}

