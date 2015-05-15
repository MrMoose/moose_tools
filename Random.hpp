
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/cstdint.hpp>

namespace moose {
namespace tools {

/*! \brief thread safe random number shortcut
 * This will give you a thread safe random number between 0 and n_max
 */
MOOSE_TOOLS_API boost::uint64_t urand(const boost::uint64_t n_max) throw ();

}
}





