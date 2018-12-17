
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MooseToolsConfig.hpp"
#include "IdTagged.hpp"
#include "Error.hpp"
#include "Carne.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>
#include <boost/container/set.hpp>
#include <boost/shared_container_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <memory>

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
		friend TaggedContainerType;

		void increment() {
			
			++m_idx;
		}

		void decrement() {

			if (m_idx > 0) {
				m_idx--;
			}
		}

		bool equal(IdTaggedContainerIterator const &n_other) const {

			return ((this->m_container == n_other.m_container)
				&& (this->m_idx == n_other.m_idx));
		}

		void advance(const std::size_t n) {
		
			m_idx += n;
		}

		typename TaggedContainerType::pointer_type dereference() const {
	
			// operator will throw on faulty index
			return (*m_container)[m_idx];
		}

		TaggedContainerType  *m_container;
		std::size_t           m_size;       // number of elements, so max idx == (m_size - 1) 
		std::size_t           m_idx;
};


/*! @brief Multi-Purpose container for id tagged types
	
	TaggedType must be derived from IdTagged.

	Modifying operations will increase incarnation count

	@note I've made this copyable but this is a shallow copy
*/
template< typename TaggedType >
class IdTaggedContainer : public Incarnated< IdTaggedContainer<TaggedType> > {

	// This container only works for types that are IdTagged
	BOOST_STATIC_ASSERT(boost::is_base_of< IdTagged< TaggedType >, TaggedType>::value);

	public:
		using pointer_type       = std::shared_ptr<TaggedType>;
		using const_pointer_type = std::shared_ptr<const TaggedType>;
		using value_type         = TaggedType;
		using const_value_type   = const TaggedType;
		using iterator           = IdTaggedContainerIterator< IdTaggedContainer< TaggedType > >;
		using const_iterator     = IdTaggedContainerIterator< const IdTaggedContainer< TaggedType > >;

		IdTaggedContainer() = default;
		IdTaggedContainer(const IdTaggedContainer &n_other) = delete;  // well, we could deep copy it...
		IdTaggedContainer(IdTaggedContainer &&n_other) noexcept {
		
			std::swap(m_objects, n_other.m_objects);
		};

		virtual ~IdTaggedContainer() noexcept = default;
		IdTaggedContainer< TaggedType > &operator=(IdTaggedContainer &&n_other) noexcept {
		
			std::swap(m_objects, n_other.m_objects);
			return *this;
		}

		/*! @brief tell if both containers contain the exact same ids
			only the ids of the objects are compared, not their actual derived content 
			@throw nil
			@return true if id are same
		*/
		bool operator==(const IdTaggedContainer &n_other) const noexcept {
		
			// easy size check first
			if (this->size() != n_other.size()) {
				return false;
			}
			
			// now see if each in here is in there too
			const objects_by_random &idx = m_objects.template get<by_random>();
			for (std::size_t i = 0; i < idx.size(); ++i) {
				if (!n_other.has(idx[i]->id())) {
					return false;
				}
			}

			// we are here and checked all and the sized are equal. This means we're done and 
			return true;
		}

		bool operator!=(const IdTaggedContainer &n_other) const noexcept {

			return !this->operator==(n_other);
		}

		/*! @brief add a new object and return if it was inserted
		  
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
				Incarnated< IdTaggedContainer<TaggedType> >::increase_incarnation();
				return true;
			}
		}

		/*! @brief insert a new object, replacing an existing one

			Objects already present will be deleted.

			@return true if object was present

			@throw internal_error on null
		*/
		bool replace(pointer_type n_object) {

			if (!n_object) {
				BOOST_THROW_EXCEPTION(internal_error() << error_message("null pointer given"));
			}

			objects_by_id &idx = m_objects.template get<by_id>();
			bool ret = (idx.erase(n_object->id()) == 1);
			insert(n_object);
			return ret;
		}


		/*! @brief remove an object by id

			@return true if object was removed
		*/
		bool remove(const typename TaggedType::id_type n_id) noexcept {

			objects_by_id &idx = m_objects.template get<by_id>();
			bool ret = (idx.erase(n_id) == 1);
			if (ret) {
				Incarnated< IdTaggedContainer<TaggedType> >::increase_incarnation();
			}

			return ret;
		}
		
		// mimic std::map erase
		iterator erase(iterator n_position) {
			
			if (n_position == end()) {
				return end();
			}

			// sadly I don't know if removal changes the order in the other index
			// I have to return an iterator to the next item. Which means, if I delete one 
			// and simply return a new iterator with the same position I cannot be sure
			// the elements after this would be the same as they were before the removal.
			// If you know this for sure, please change accordingly.
			objects_by_random &ridx = m_objects.template get<by_random>();
			typename objects_by_random::iterator rai = ridx.begin() + n_position.m_idx;

			typename objects_by_id::iterator deli = m_objects.template project<by_id>(rai);
			objects_by_id &oidx = m_objects.template get<by_id>();
			oidx.erase(deli);
			Incarnated< IdTaggedContainer<TaggedType> >::increase_incarnation();
			return iterator(this) + n_position.m_idx;
		}

		void clear() noexcept {
	
			if (size()) {
				m_objects.clear();
				Incarnated< IdTaggedContainer<TaggedType> >::increase_incarnation();
			}
		}

		//! Is there one with that id?
		bool has(const typename TaggedType::id_type n_id) const noexcept {

			const objects_by_id &idx = m_objects.template get<by_id>();
			return idx.count(n_id) > 0;
		}
		
		//! Is there one with that id?
		bool has(const TaggedType &n_object) const noexcept {

			const objects_by_id &idx = m_objects.template get<by_id>();
			return idx.count(n_object.id()) > 0;
		}

		//! How did I get this long without actually retrieving things?
		//! @return null on not found
		pointer_type get(const typename TaggedType::id_type n_id) const noexcept {

			const objects_by_id &idx = m_objects.template get<by_id>();
			typename objects_by_id::const_iterator i = idx.find(n_id);
			if (i != idx.end()) {
				return *i;
			} else {
				return pointer_type();
			}
		}

		//! how many are in there?
		std::size_t size() const noexcept {

			return m_objects.template get<by_id>().size();
		}

		pointer_type operator[](const std::size_t n_idx) {

			if (n_idx >= size()) {
				BOOST_THROW_EXCEPTION(internal_error() << error_message("container index out of bounds")
					<< error_argument(n_idx));
			}
			// would it be better to check for out of bounds rather than let the idx throw?
			objects_by_random &idx = m_objects.template get<by_random>();
			return idx[n_idx];
		}

		const pointer_type operator[](const std::size_t n_idx) const {

			if (n_idx >= size()) {
				BOOST_THROW_EXCEPTION(internal_error() << error_message("container index out of bounds")
					<< error_argument(n_idx));
			}
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

		bool empty() const noexcept {

			const objects_by_random &idx = m_objects.template get<by_random>();
			return idx.empty();
		}

		const_iterator cend() const {

			return const_iterator(this) + size();
		};

		//! @throw std::bad_alloc
		boost::container::set<typename TaggedType::id_type> ids_in_container() const {
			
			boost::container::set<typename TaggedType::id_type> ret;
			const objects_by_random &idx = m_objects.template get<by_random>();
			for (std::size_t i = 0; i < idx.size(); ++i) {
				ret.insert(idx[i]->id());
			}
			return ret;
		};

	private:

		struct by_id {};
		struct by_random {};

		using tagged_container_type = boost::multi_index_container<
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
				>;
		
		using objects_by_id     = typename tagged_container_type::template index<by_id>::type;
		using objects_by_random = typename tagged_container_type::template index<by_random>::type;

		tagged_container_type  m_objects;
};

#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void IdTaggedContainerGetRidOfLNK4221();
#endif

} // namespace tools
} // namespace moose
