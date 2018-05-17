
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/cstdint.hpp>
#include <boost/uuid/uuid.hpp>

namespace moose {
namespace tools {

/*! @brief thread safe random number shortcut
 * This will give you a thread safe random number between 0 and n_max
 *  @throw std::bad_alloc when out of memory on first use
 */
MOOSE_TOOLS_API boost::uint64_t urand(const boost::uint64_t n_max);

/*! @brief thread safe random number shortcut
 * This will give you a thread safe random number between 0 and std::numerical_limits<uint64_t>::max()
 *  @throw std::bad_alloc when out of memory on first use
 */
MOOSE_TOOLS_API boost::uint64_t urand();

/*! @brief creates a random uuid out of thin air
 *  @return random uuid
 *  @throw std::bad_alloc when out of memory on first use
 */
MOOSE_TOOLS_API boost::uuids::uuid ruuid(void);

/*! @brief supposedly the fasted.
	Found this at https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
	and just took it here. All credits to original author
 */
MOOSE_TOOLS_API unsigned long xorshf96(void);

}
}





