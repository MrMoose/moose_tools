
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE ThreadIdTests
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include "../ThreadId.hpp"

#if defined(BOOST_MSVC)
#pragma warning (disable : 4553) // faulty '==': operator has no effect; did you intend '='?  in checks
#endif

#include <set>

BOOST_AUTO_TEST_CASE(RunOnce) {
	
	BOOST_CHECK(moose::tools::faked_thread_id() != 0);
}

void insert_own_id(boost::mutex &n_mutex, std::set<unsigned int> &n_set) {
	
	unsigned int id_here = moose::tools::faked_thread_id();
	boost::unique_lock<boost::mutex> slock(n_mutex);
	n_set.insert(id_here);
}

BOOST_AUTO_TEST_CASE(RunThreaded) {
	
	// do this in 100 new threads and assume the results mustn't be identical
	boost::mutex m;
	std::set<unsigned int> results;
	
	boost::thread_group threads;
	for (unsigned int i = 0; i < 100; ++i) {
		// all 100 threads put their ID in the set
		threads.create_thread(boost::bind(&insert_own_id, boost::ref(m), boost::ref(results)));
	}
	
	// wait for them to finish
	threads.join_all();
	
	// now we must have 100 ids
	BOOST_CHECK(results.size() == 100);
}


