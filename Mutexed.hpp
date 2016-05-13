
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/thread/mutex.hpp>

namespace moose {
namespace tools {

/*! \brief shortcut base for classes with a mutex
 */
template< typename DerivedType >
class Mutexed {

	public:
		typedef boost::mutex::scoped_lock scoped_lock;
		scoped_lock lock(void) const {
			
			return boost::mutex::scoped_lock(m_mutex);
		}
		
	protected:
		Mutexed(void) = default;
		Mutexed(const Mutexed &n_other) = delete;
		virtual ~Mutexed() noexcept = default;

	private:
		mutable boost::mutex m_mutex;
};

#if BOOST_MSVC
MOOSE_TOOLS_API void MutexedgetRidOfLNK4221();
#endif

}
}

