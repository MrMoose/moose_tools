
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Random.hpp"
#include "ThreadId.hpp"

#include <boost/thread/tss.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace moose {
namespace tools {

namespace {
	
	typedef boost::random::mt19937                             prng_type;
	boost::thread_specific_ptr<prng_type>                      local_gen;
	boost::thread_specific_ptr<boost::uuids::random_generator> random_uid_generator;
	
	/// get access to a thread local instance of the PRNG
	inline prng_type* gen() {
		
		if (!local_gen.get()) {
			
			// I try to get a high resolution nanosecond clock and taint 
			// it somehow with some burned time
			boost::chrono::high_resolution_clock::time_point start = boost::chrono::high_resolution_clock::now();
			unsigned int tid = faked_thread_id(); // this should take long enough...
			boost::chrono::nanoseconds ns = boost::chrono::high_resolution_clock::now() - start;
			local_gen.reset(new prng_type(ns.count() % tid));
		}
		
		return local_gen.get();
	}
	
	// get access to a thread local instance of a uuid generator
	inline boost::uuids::random_generator *get_uuid_generator(void) {
		
		if (!random_uid_generator.get()) {
			prng_type *prng = gen();
			random_uid_generator.reset(new boost::uuids::random_generator(*prng));
		}
		
		return random_uid_generator.get();
	}
	
}

boost::uint64_t urand(const boost::uint64_t n_max) {

	// get the thread local PRNG
	boost::random::mt19937 *prng = gen();
	assert(prng);
	boost::random::uniform_int_distribution<boost::uint64_t> dist(0, n_max);
	return dist(*prng);
}

boost::uint64_t urand(void) {

	return moose::tools::urand(std::numeric_limits<boost::uint64_t>::max());
}

boost::uuids::uuid ruuid(void) {

	boost::uuids::random_generator *gen = get_uuid_generator();
	return (*gen)();
};

}
}

