
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
		struct scoped_lock : public boost::unique_lock< Lockable > {
	
			scoped_lock() = delete;
			scoped_lock(Mutexed<DerivedType> &n_issuer, bool n_writing)
					: boost::unique_lock<Lockable>(n_issuer.m_mutex)
					, m_issuer(n_issuer)
					, m_writing(n_writing) {};
			scoped_lock(const scoped_lock &) = delete;
			scoped_lock(scoped_lock &&) = default;
			~scoped_lock() noexcept {
				if (m_writing) {
					m_issuer.m_incarnation++;
				}
			};

			//! upgrade a read_lock to a write_lock
			void upgrade() noexcept {
				m_writing = true;
			}

			//! downgrade a write_lock to a read lock
			void downgrade() noexcept {
				m_writing = false;
			}

			Mutexed<DerivedType> &m_issuer;
			bool                  m_writing;
		};

		scoped_lock acquire_read_lock(void) const {
			
			return scoped_lock(*const_cast< Mutexed< DerivedType > *>(this), false);
		}

		scoped_lock acquire_write_lock(void) {
			
			return scoped_lock(*this, true);
		}
		
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

#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void MutexedgetRidOfLNK4221();
#endif

}
}

