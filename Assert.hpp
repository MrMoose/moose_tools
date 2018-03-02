
//  Copyright 2017 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#ifdef MOOSE_DEBUG

#include <boost/config.hpp> // for BOOST_LIKELY
#include <boost/current_function.hpp>

#include <iostream>

/*! \brief release mode assertions
	Sometimes you would like to use assertions in release builds.
	Those macros make the assertion depending on the presence of
	compiler flag -DMOOSE_DEBUG and not on _DEBUG or NDEBUG
 */
#define MOOSE_ASSERT(expr)                                             \
	if (!expr) {                                                       \
		std::cerr << "Assertion failed at " << BOOST_CURRENT_FUNCTION  \
			<< " in " << __FILE__ << ":" << __LINE__ << std::endl;     \
		std::terminate();                                              \
    }                                                                  \

#define MOOSE_ASSERT_MSG(expr, msg)                                    \
	if (!expr) {                                                       \
		std::cerr << "Assertion failed!\n"                             \
			<< msg << "\nat " << BOOST_CURRENT_FUNCTION                \
			<< " in " << __FILE__ << ":" << __LINE__ << std::endl;     \
		std::terminate();                                              \
    }                                                                  \

#else

#define MOOSE_ASSERT(expr) ((void)0)
#define MOOSE_ASSERT_MSG(expr, msg) ((void)0)

#endif

#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void AssertgetRidOfLNK4221();
#endif
