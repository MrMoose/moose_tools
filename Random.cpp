
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Random.hpp"
#include "ThreadId.hpp"
#include "Assert.hpp"

#include <boost/thread/tss.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace moose {
namespace tools {

namespace {
	
	using prng_type = boost::random::mt19937;
	boost::thread_specific_ptr<prng_type>                      local_gen;
	using uuid_generator_type = boost::uuids::basic_random_generator<prng_type>;
	boost::thread_specific_ptr<uuid_generator_type>            random_uid_generator;
	
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
	inline uuid_generator_type *get_uuid_generator() {
		
		if (!random_uid_generator.get()) {
			prng_type *prng = gen();
			random_uid_generator.reset(new uuid_generator_type(*prng));
		}
		
		return random_uid_generator.get();
	}
	
}

boost::uint64_t urand(const boost::uint64_t n_max) {

	// get the thread local PRNG
	boost::random::mt19937 *prng = gen();
	MOOSE_ASSERT(prng);
	boost::random::uniform_int_distribution<boost::uint64_t> dist(0, n_max);
	return dist(*prng);
}

boost::uint64_t urand(const boost::uint64_t n_min, const boost::uint64_t n_max) {
	
	MOOSE_ASSERT_MSG((n_min < n_max), "minimum value must be lower than maximum value when calling moose::tools::urand()");

	// get the thread local PRNG
	boost::random::mt19937 *prng = gen();
	MOOSE_ASSERT(prng);
	boost::random::uniform_int_distribution<boost::uint64_t> dist(n_min, n_max);
	return dist(*prng);
}

boost::uint64_t urand() {

	return moose::tools::urand(std::numeric_limits<boost::uint64_t>::max());
}

boost::uuids::uuid ruuid() {

	uuid_generator_type *gen = get_uuid_generator();
	return (*gen)();
};


static unsigned long x = 123456789;
static unsigned long y = 362436069;
static unsigned long z = 521288629;

unsigned long xorshf96() {          // period 2^96-1

	unsigned long t;
	x ^= x << 16;
	x ^= x >> 5;
	x ^= x << 1;

	t = x;
	x = y;
	y = z;
	z = t ^ x ^ y;

	return z;
}

}
}

