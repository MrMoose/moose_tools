
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <stdexcept>
#include <cassert>

namespace moose {
namespace tools {

//! todo! Change this to CRTP to avoid vtable?
struct Pimplee {

	//! note that your impl must implement that
	RTT_PHYSIK_CORE_API virtual ~Pimplee() throw ();
};

//! PimplType must be default constructable
//! and deriving from struct Pimplee
template< typename PimplType >
class Pimpled {

	protected:
		Pimpled(void)
			: m_d(new PimplType()) {

			// Check if our PimplType has the correct base
			if (!dynamic_cast<Pimplee *>(m_d)) {
				throw std::runtime_error("incorrect pimpl base class");
			}
		}

		~Pimpled(void) throw () {

			Pimplee *tmp = reinterpret_cast<Pimplee *>(m_d);
			assert(tmp);
			delete tmp;
			m_d = nullptr;
		}

		PimplType &d(void) {
			// If you see this error message, you have not called the base c'tor 
			// Pimpled<yourType>() in your c'tors initializer list.
			assert(m_d);
			return *m_d;
		}
		
		PimplType const &d(void) const {
			// If you see this error message, you have not called the base c'tor 
			// Pimpled<yourType>() in your c'tors initializer list.
			assert(m_d);
			return *m_d;
		}
	
	private:
		PimplType *m_d;
};

}
}

