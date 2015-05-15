
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ThreadId.hpp"

#include <boost/thread/thread.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/functional/hash.hpp>

#include <sstream>
#include <string>

namespace moose {
namespace tools {

unsigned int faked_thread_id() throw () {
	
	using namespace boost::spirit;
	
	try {
		unsigned int ret = 0;
		std::stringstream tmpo;
		tmpo << boost::this_thread::get_id();
		std::string s(tmpo.str());
		std::string::const_iterator begin = s.begin();
		std::string::const_iterator end = s.end();
		bool r = qi::parse(begin, end, qi::uint_, ret);
		if (r && (begin == end)) {
			// the thread id parsed successfully to something resembling a number
			return ret;
		} else {
			// it did not resemble a number. Maybe I can hash it...
			boost::hash<std::string> string_hash;
			return string_hash(s);
		}
		
	} catch (const boost::thread_resource_error &e) {
		// what shall I return here?
		uintptr_t ret = reinterpret_cast<uintptr_t>(&e);
		return ret;
	}
}

}
}

