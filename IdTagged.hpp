
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "Random.hpp"

#include <boost/cstdint.hpp>

namespace moose {
namespace tools {

//! Give a class an ID which is randomly generated in c'tor
//! which can not be changed afterwards
template< typename DerivedType >
class IdTagged {

	protected:
		//! Note that this c'tor can throw but only std::bad_alloc, which all new can
		//! \throw std::bad_alloc when out of memory on first use
		IdTagged(void)
			: m_id(moose::tools::urand()) {
		}

		//! you can also give in the ID of course
		IdTagged(const boost::uint64_t n_id)
			: m_id(n_id) {
		}
		
		~IdTagged(void) noexcept {}

		boost::uint64_t id(void) const noexcept {
			
			return m_id;
		}
	
	private:
		const boost::uint64_t m_id;
};

}
}

