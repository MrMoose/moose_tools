
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Mutexed.hpp"

namespace moose {
namespace tools {

#if BOOST_MSVC
void MutexedgetRidOfLNK4221() {}
#endif

}
}
