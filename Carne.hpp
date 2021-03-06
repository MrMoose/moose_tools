
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "Random.hpp"
#include "Assert.hpp"

#include <boost/cstdint.hpp>
#include <boost/type_traits/is_same.hpp>

#include <atomic>

namespace moose {
namespace tools {

class IncarnatedUnusedParent {};

/*! @brief Give a class an atomic incarnation counter 
 *  
 *  You may also provide a parent, causing an incarnation increase of this to also increase the parent object's.
 */
template< typename DerivedType, typename ParentType = IncarnatedUnusedParent >
class Incarnated {

	protected:
		//! Note that this c'tor can throw but only std::bad_alloc, which all new can
		//! @throw std::bad_alloc when out of memory on first use
		Incarnated()
				: m_incarnation(0) {
		}

		//! When deserializing such objects from somewhere you may need a c'tor which
		//! Initialized the object with a given incarnation
		Incarnated(const boost::int64_t n_incarnation)
				: m_incarnation{ static_cast<boost::uint64_t>(n_incarnation) } {
		}

		//! Note that this c'tor can throw but only std::bad_alloc, which all new can
		//! @throw std::bad_alloc when out of memory on first use
		Incarnated(const ParentType *n_parent)
				: m_incarnation(0)
				, m_parent(n_parent) {

		}

		/*! @brief you can also give in the ID of course but only in protected c'tor
		 *   so the derived class decides whether to offer this possibility but only
		 *   at creation time
		 */
		Incarnated(const Incarnated &n_other)
				: m_incarnation(n_other.m_incarnation) {
		}

		~Incarnated() noexcept = default;

		/*! @brief hand in a parent object.
			Without it the class will assert when increasing inc
		 */
		void set_parent(const ParentType *n_parent) {

			m_parent(n_parent);
			MOOSE_ASSERT(m_parent);
		}

	public:
		//! this is the main self-explanatory getter
		boost::uint64_t incarnation() const noexcept {
			
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

			MOOSE_ASSERT(m_parent);
			++m_incarnation;
			m_parent->increase_incarnation();
		}

		std::atomic<boost::uint64_t>  m_incarnation;
		ParentType                   *m_parent = nullptr;
};


#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void CarneGetRidOfLNK4221();
#endif

}
}

