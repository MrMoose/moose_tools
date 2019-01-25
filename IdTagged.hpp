
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "Random.hpp"
#include "Assert.hpp"

#include <boost/cstdint.hpp>

#include <memory>
#include <limits>

namespace moose {
namespace tools {

/*! @brief Give a class an ID which is randomly generated in c'tor
 *  which can not be changed afterwards
 *  @todo consider making this noncopyable for semantics
 */
template< typename DerivedType, typename IdType = boost::uint64_t>
class IdTagged {

	public:
		using id_type = IdType;

	protected:
		//! Create new object with random non-null ID
		//! @throw std::bad_alloc when out of memory on first use
		IdTagged()
				: m_id(moose::tools::urand(1, std::numeric_limits< id_type >::max())) {
		}

		//! This c'tor allows to specify an id within a given range
		//! @throw std::bad_alloc when out of memory on first use
		IdTagged(const id_type n_id_min, const id_type n_id_max)
				: m_id(moose::tools::urand(n_id_min, n_id_max)) {

		}

		/*! @brief you can also give in the ID of course but only in protected c'tor
		 *   so the derived class decides whether to offer this possibility but only
		 *   at creation time
		 */
		IdTagged(const id_type n_id)
				: m_id(n_id) {
		}

		IdTagged(const IdTagged &n_other) = delete;
		~IdTagged() noexcept = default;

	public:
		//! this is the main self-explanatory getter
		id_type id() const noexcept {
			
			return m_id;
		}

		/*! @brief multi-purpose key extractor
			And this is a key extractor function for use in containers such as 
			multi-index
		 */
		typedef struct tag_id_extractor {

			using result_type = id_type;

			const result_type operator()(const IdTagged< DerivedType > &n_o) const noexcept {
				return n_o.id();
			}

			result_type operator()(IdTagged< DerivedType > &n_o) const noexcept {
				return n_o.id();
			}

			const result_type operator()(const std::shared_ptr< IdTagged< DerivedType > > &n_o) const noexcept {
				MOOSE_ASSERT(n_o);
				return n_o->id();
			}

			result_type operator()(std::shared_ptr< IdTagged< DerivedType > > &n_o) const noexcept {
				MOOSE_ASSERT(n_o);
				return n_o->id();
			}

		} IdExtractor;
	
	private:
		const id_type m_id;
};


#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void IdTaggedgetRidOfLNK4221();
#endif

}
}

