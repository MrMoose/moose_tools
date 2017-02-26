
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "Random.hpp"

#include <boost/cstdint.hpp>
#include <boost/atomic.hpp>
#include <boost/type_traits/is_same.hpp>

namespace moose {
namespace tools {

class IncarnatedUnusedParent {};

/*! \brief Give a class an atomic incarnation counter 
 *  
 *  You may also provide a parent, causing an incarnation increase of this to also increase the parent object's.
 */
template< typename DerivedType, typename ParentType = IncarnatedUnusedParent >
class Incarnated {

	// This container only works for types that are IdTagged
	//BOOST_STATIC_ASSERT(boost::is_base_of< Incarnated< ParentType >, ParentType>::value);

	protected:
		//! Note that this c'tor can throw but only std::bad_alloc, which all new can
		//! \throw std::bad_alloc when out of memory on first use
		Incarnated(void)
				: m_incarnation(0) {
		}

		//! Note that this c'tor can throw but only std::bad_alloc, which all new can
		//! \throw std::bad_alloc when out of memory on first use
		Incarnated(const ParentType *n_parent)
			: m_incarnation(0)
			, m_parent(n_parent) {

		}

		/*! \brief you can also give in the ID of course but only in protected c'tor
		 *   so the derived class decides whether to offer this possibility but only
		 *   at creation time
		 */
		Incarnated(const Incarnated &n_other)
				: m_incarnation(n_other.m_incarnation) {
		}

		~Incarnated(void) noexcept = default;

		/*! \brief hand in a parent object.
			Without it the class will assert when increasing inc
		 */
		void set_parent(const ParentType *n_parent) {

			m_parent(n_parent);
			assert(m_parent);
		}

	public:
		//! this is the main self-explanatory getter
		boost::uint64_t incarnation(void) const noexcept {
			
			return m_incarnation.load();
		}

		/// implementation to increase incarnation chooses if the parent will be increased as well
		void increase_incarnation() noexcept {
		
			return increase_incarnation_impl(typename boost::is_same<ParentType, IncarnatedUnusedParent>::type());
		}
	
		bool changed(const boost::uint64_t n_known_incarnation) const noexcept {
		
			return (m_incarnation.load() > n_known_incarnation);
		}

	private:

		//! This implementation is for classes that didn't specify a parent type
		inline void increase_incarnation_impl(boost::true_type) noexcept {

			++m_incarnation;
		}

		//! This implementation is for classes which did specify a parent type
		inline void increase_incarnation_impl(boost::false_type) noexcept {

			assert(m_parent);

			++m_incarnation;
			m_parent->increase_incarnation();
		}

		boost::atomic<boost::uint64_t>  m_incarnation;

		ParentType               *m_parent = nullptr;
};


#if BOOST_MSVC
MOOSE_TOOLS_API void CarneGetRidOfLNK4221();
#endif

}
}

