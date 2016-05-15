
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

template< class TaggedContainerType >
class IdTaggedContainerIterator
		: public boost::iterator_facade<
					IdTaggedContainerIterator< TaggedContainerType >,    // CRTP derived
					typename TaggedContainerType::pointer_type,          // iterator's value_type
					boost::random_access_traversal_tag,                  // iterator capabilities model
					typename TaggedContainerType::pointer_type           // reference type 
				> {

	public:
		IdTaggedContainerIterator()
				: m_container(nullptr) 
				, m_size(0)
				, m_idx(static_cast<std::size_t>(-1)) {
		}

		//! yes, null is BS here
		explicit IdTaggedContainerIterator(TaggedContainerType *n_container)
				: m_container(n_container)
				, m_size(m_container->size())
				, m_idx(0) {
		}

		IdTaggedContainerIterator(const IdTaggedContainerIterator &n_other) = default;
		~IdTaggedContainerIterator() = default;

	private:
		//! those are being used by boost iterator and need friend access
		friend class boost::iterator_core_access;

		void increment() {

			if (m_idx < (m_size - 1)) {
				++m_idx;
			} else {
				// I make this end, but keep the container
				m_idx = static_cast<std::size_t>(-1);
			}
		}

		void decrement() {
		
			if (m_idx != 0) {
				m_idx--;
			}
		}

		bool equal(IdTaggedContainerIterator const &n_other) const {

			return ((this->m_container == n_other.m_container)
				&& (this->m_idx == n_other.m_idx));
		}

		void advance(const std::size_t n) {
		
			std::size_t new_idx = m_idx + n;
			if (new_idx < 0) {
				new_idx = 0;
			} else if (new_idx >= m_size) {
				new_idx = static_cast<std::size_t>(-1);
			}
			m_idx = new_idx;
		}

		typename TaggedContainerType::pointer_type dereference() const {
			
			return (*m_container)[m_idx];
		}


		TaggedContainerType  *m_container;
		std::size_t           m_size;       // number of elements, so max idx == (m_size - 1) 
		std::size_t           m_idx;
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
		typedef TaggedType                          value_type;
		typedef const TaggedType                    const_value_type;
		typedef IdTaggedContainerIterator< IdTaggedContainer< TaggedType > > iterator;
		typedef IdTaggedContainerIterator< const IdTaggedContainer< TaggedType > > const_iterator;

		IdTaggedContainer() = default;
		IdTaggedContainer(const IdTaggedContainer &n_other) = delete;  // well, we could deep copy it...
		IdTaggedContainer(IdTaggedContainer &&n_other) = default;
		virtual ~IdTaggedContainer() noexcept = default;

		/*! @brief add a new object and return its id
		  
			Objects already present will not be inserted

			@throw internal_error on null
			@return true if object was added
		 */
		bool insert(pointer_type n_object) {
			
			if (!n_object) {
				BOOST_THROW_EXCEPTION(internal_error() << error_message("null pointer given"));
			}
			
			objects_by_id &idx = m_objects.template get<by_id>();
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
		bool remove(const typename TaggedType::id_type n_id) noexcept {

			objects_by_id &idx = m_objects.template get<by_id>();
			return idx.erase(n_id) == 1;
		}
		
		//! how many are in there?
		bool has(const typename TaggedType::id_type n_id) const noexcept {

			const objects_by_id &idx = m_objects.template get<by_id>();
			return idx.count(n_id) > 0;
		}

		//! how many are in there?
		std::size_t size() const noexcept {

			return m_objects.template get<by_id>().size();
		}

		pointer_type operator[](const std::size_t n_idx) {

			assert((n_idx < size()) && (n_idx >= 0));
			// would it be better to ceck for out of bounds rather than let the idx throw?
			objects_by_random &idx = m_objects.template get<by_random>();
			return idx[n_idx];
		}

		const pointer_type operator[](const std::size_t n_idx) const {

			assert((n_idx < size()) && (n_idx >= 0));
			const objects_by_random &idx = m_objects.template get<by_random>();
			return idx[n_idx];
		}

		iterator begin() {

			return iterator(this);
		};

		const_iterator begin() const {

			return cbegin();
		};

		const_iterator cbegin() const {

			return const_iterator(this);
		};

		iterator end() {

			return iterator(this) + size();
		};

		const_iterator end() const {

			return cend();
		};

		const_iterator cend() const {

			return const_iterator(this) + size();
		};

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
		
		typedef typename tagged_container_type::template index<by_id>::type     objects_by_id;
		typedef typename tagged_container_type::template index<by_random>::type objects_by_random;

		tagged_container_type  m_objects;
};



#if BOOST_MSVC
MOOSE_TOOLS_API void IdTaggedContainerGetRidOfLNK4221();
#endif

} // namespace tools
} // namespace moose
