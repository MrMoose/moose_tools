
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Random.hpp"
#include "ThreadId.hpp"

#include <boost/thread/tss.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace moose {
namespace tools {

namespace {

	boost::thread_specific_ptr<boost::random::mt19937> local_gen;
	
	/// get access to a thread local instance of the PRNG
	inline boost::random::mt19937* gen() {
		
		if (!local_gen.get()) {
			
			// I try to get a high resolution nanosecond clock and taint 
			// it somehow with some burned time
			boost::chrono::high_resolution_clock::time_point start = boost::chrono::high_resolution_clock::now();
			unsigned int tid = faked_thread_id(); // this should take long enough...
			boost::chrono::nanoseconds ns = boost::chrono::high_resolution_clock::now() - start;
			local_gen.reset(new boost::random::mt19937(ns.count() % tid));
		}
		
		return local_gen.get();
	}
}

boost::uint64_t urand(const boost::uint64_t n_max) throw () {

	// get the thread local PRNG
	boost::random::mt19937 *prng = gen();
	assert(prng);
	boost::random::uniform_int_distribution<> dist(0, n_max);
	return dist(*prng);
}

}
}

