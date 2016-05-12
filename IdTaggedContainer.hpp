
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MooseToolsConfig.hpp"
#include "IdTagged.hpp"
#include "Error.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/shared_container_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace moose {
namespace tools {

	
template< typename TaggedContainerType >
class IdTaggedContainerIterator
		: public boost::iterator_facade<
				IdTaggedContainerIterator< typename TaggedContainerType::value_type >,
					TaggedContainerType,
					boost::bidirectional_traversal_tag,
					boost::use_default             // reference type is TaggedContainerType::value_type& by default
				> {


};


/*! \brief Multi-Purpose container for id tagged types
 */
template< typename TaggedType >
class IdTaggedContainer {

	// This container only works for types that are IdTagged
	BOOST_STATIC_ASSERT(boost::is_base_of< IdTagged< TaggedType >, TaggedType>::value);

	public:
		typedef boost::shared_ptr<TaggedType>       pointer_type;
		typedef boost::shared_ptr<const TaggedType> const_pointer_type;
		typedef typename TaggedType                 value_type;
		typedef typename const TaggedType           const_value_type;

		IdTaggedContainer() = default;
		IdTaggedContainer(const IdTaggedContainer &n_other) = delete;  // well, we could deep copy it...
		virtual ~IdTaggedContainer() noexcept = default;

		/*! @brief add a new object and return its id
		  
			Objects already present will not be inserted

			@throw internal_error on null
			@return true if object was added
		 */
		bool insert(pointer_type n_object) {
			
			typedef typename tagged_container_type::index<by_id>::type objects_by_id;

			if (!n_object) {
				BOOST_THROW_EXCEPTION(internal_error() << error_message("null pointer given"));
			}
			
			objects_by_id &idx = m_objects.get<by_id>();
			if (idx.count(n_object->id())) {
				// object already present
				return false;
			} else {
				idx.insert(n_object);
				return true;
			}
		}

		/*! @brief remove an object by id

			@return true if object was removed
		*/
		bool remove(const boost::uint64_t n_id) noexcept {

			typedef typename tagged_container_type::index<by_id>::type objects_by_id;

			objects_by_id &idx = m_objects.get<by_id>();
			return idx.erase(n_id) == 1;
		}
		
		//! how many are in there?
		std::size_t size() const noexcept {

			return m_objects.get<by_id>().size();
		}

		//value_type &operator[](const std::size_t n_idx) {
		
			
		//}

	private:

		struct by_id {};
		struct by_random {};

		typedef boost::multi_index_container<
					pointer_type,
					boost::multi_index::indexed_by<
						boost::multi_index::ordered_unique<
							boost::multi_index::tag<by_id>,
							typename TaggedType::IdExtractor
						>,
						boost::multi_index::random_access<
							boost::multi_index::tag<by_random>
						>
					>
				> tagged_container_type;

		tagged_container_type  m_objects;
};



#if BOOST_MSVC
MOOSE_TOOLS_API void IdTaggedContainerGetRidOfLNK4221();
#endif

} // namespace tools
} // namespace moose
