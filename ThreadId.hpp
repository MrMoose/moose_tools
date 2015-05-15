
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

namespace moose {
namespace tools {

//! This assumes the the thread ID on your system somehow rhymes with 
//! a number and tries hard to give you that number
MOOSE_TOOLS_API unsigned int faked_thread_id() throw ();

}
}





