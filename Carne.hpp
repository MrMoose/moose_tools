
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "Random.hpp"

#include <boost/cstdint.hpp>
#include <boost/atomic.hpp>

namespace moose {
namespace tools {

/*! \brief Give a class an atomic incarnation counter 
 * 
 */
template< typename DerivedType>
class Incarnated {

	protected:
		//! Note that this c'tor can throw but only std::bad_alloc, which all new can
		//! \throw std::bad_alloc when out of memory on first use
		Incarnated(void)
				: m_incarnation(0) {
		}

		/*! \brief you can also give in the ID of course but only in protected c'tor
		 *   so the derived class decides whether to offer this possibility but only
		 *   at creation time
		 */
		Incarnated(const Incarnated &n_other)
				: m_incarnation(n_other.m_incarnation) {
		}

		~Incarnated(void) noexcept = default;

	public:
		//! this is the main self-explanatory getter
		boost::uint64_t incarnation(void) const noexcept {
			
			return m_incarnation.load();
		}

		void increase_incarnation() noexcept {
		
			++m_incarnation;
		}
	
		bool changed(const boost::uint64_t n_known_incarnation) const noexcept {
		
			return (m_incarnation.load() > n_known_incarnation);
		}

	private:
		boost::atomic<boost::uint64_t>  m_incarnation;
};


#if BOOST_MSVC
MOOSE_TOOLS_API void CarneGetRidOfLNK4221();
#endif

}
}

