
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
	Pimplee() = default;
	Pimplee(const Pimplee &n_other) = delete;
	MOOSE_TOOLS_API virtual ~Pimplee() noexcept;
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
				delete m_d;
				m_d = nullptr;
				throw std::runtime_error("incorrect pimpl base class");
			}
		}

		//! copying this would imply a deep copy of pimpl
		//! which we could do but I'd rather not force 
		//! the user to make PimplType copyable.
		//! Is there a better way to customize this?
		Pimpled(const Pimpled &n_other) = delete;
		~Pimpled(void) noexcept {

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
		PimplType *m_d = nullptr;
};

}
}

