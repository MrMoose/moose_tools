
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/cstdint.hpp>

namespace moose {
namespace tools {

/*! \brief shortcut base for classes with a mutex
 */
template< typename DerivedType, typename Lockable = boost::mutex >
class Mutexed {

	public:
	
		/*! \brief this is basically a regular unique_lock which
		 * increases the locked object's incarnation count upon release.
		 * Of course this doesn't imply any data have actually been changed
		 * but it signals the possibility thereof.
		 */
		struct write_lock : public boost::unique_lock< Lockable > {
	
			write_lock(Mutexed<DerivedType> &n_issuer)
					: boost::unique_lock<Lockable>(n_issuer.m_mutex)
					, m_issuer(n_issuer) {};
			write_lock(const write_lock &) = delete;
			write_lock(write_lock &&) = default;
			~write_lock() noexcept {
				m_issuer.m_incarnation++;
			};
			Mutexed<DerivedType> &m_issuer;
		};

		//! a regular unique_lock which doesn't modify incarnation
		struct read_lock : public boost::unique_lock< Lockable > {
	
			read_lock(const Mutexed<DerivedType> &n_issuer)
					: boost::unique_lock<Lockable>(n_issuer.m_mutex) {};
			read_lock(const read_lock &) = delete;
			read_lock(read_lock &&) = default;
			~read_lock() noexcept = default;
		};


		read_lock acquire_read_lock(void) const {
			
			return read_lock(*this);
		}

		write_lock acquire_write_lock(void) {
			
			return write_lock(*this);
		}

		
	//	scoped_lock lock(void) const {
			
	//		return boost::mutex::scoped_lock(m_mutex);
	//	}
		
		boost::uint64_t incarnation() const noexcept {
		
			return m_incarnation;
		}

	protected:
		// I start with incarnation 1 rather than 0 to mark 0 as overflow case
		Mutexed(void) : m_incarnation(1) {};
		Mutexed(const Mutexed &n_other) = delete;
		virtual ~Mutexed() noexcept = default;

	private:
		mutable Lockable     m_mutex;
		boost::uint64_t      m_incarnation;
};

#if BOOST_MSVC
MOOSE_TOOLS_API void MutexedgetRidOfLNK4221();
#endif

}
}

