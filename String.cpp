
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "String.hpp"

namespace moose {
namespace tools {

void truncate(std::string &n_string, std::size_t n_length) noexcept {

	if (n_string.length() > n_length) {
		n_string.resize(n_length);
	}
}

void getRidOfLNK4221() {}

}
}