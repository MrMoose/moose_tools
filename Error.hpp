
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MooseToolsConfig.hpp"

#include <boost/exception/all.hpp>
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include <boost/system/error_code.hpp>

// needed for MSVC - Uses COM to show debug info
#if defined(BOOST_MSVC)
	#define BOOST_STACKTRACE_USE_WINDBG 
#endif
#include <boost/stacktrace.hpp>

#include <string>
#include <sstream>

namespace moose {
namespace tools {

MOOSE_TOOLS_API std::string get_last_error();
MOOSE_TOOLS_API void set_last_error(const std::string &n_error_message);
	
struct MOOSE_TOOLS_API moose_error: virtual std::exception, virtual boost::exception {

	public:
		MOOSE_TOOLS_API moose_error();
		MOOSE_TOOLS_API virtual char const *what() const noexcept override;
		MOOSE_TOOLS_API virtual ~moose_error() noexcept { }
};


//! An exception that is only to be used in unit tests. Allows 
//! for separation of test specific exceptions and 'real' ones
struct MOOSE_TOOLS_API unit_test_error : virtual moose_error {
	
	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! Some component violated protocol specifications and talked BS
struct MOOSE_TOOLS_API protocol_error : virtual moose_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! Unspecified internal fuckup
struct MOOSE_TOOLS_API internal_error : virtual moose_error {
	
	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! UI structuring internal error
struct MOOSE_TOOLS_API ui_error : virtual internal_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! Generic IO error
struct MOOSE_TOOLS_API io_error : virtual moose_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! Serialization failed (not impl specific)
struct MOOSE_TOOLS_API serialization_error : virtual io_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! Disk IO failed
struct MOOSE_TOOLS_API file_error : virtual io_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};

//! Network IO failed
struct MOOSE_TOOLS_API network_error : virtual io_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept override;
};


//! tag any exception with an error code
using errno_code = boost::error_info<struct tag_errno_code, int>;

//! tag any exception with a human readable error message
using error_message = boost::error_info<struct tag_error_message, std::string>;

//! tag protocol exceptions with an request / response id
using request_id = boost::error_info<struct tag_request_id, boost::uint64_t>;

//! tag exceptions with an session id
using session_id = boost::error_info<struct tag_session_id, boost::uint64_t>;

//! also a boost system error code
using error_code = boost::error_info<struct tag_error_code, boost::system::error_code>;

//! The argument that led to the error
using error_argument_type = boost::error_info<struct tag_error_argument, std::string>;

//! tag exceptions with stacktrace info
using error_stacktrace = boost::error_info<struct tag_stacktrace_dump, boost::stacktrace::stacktrace>;

//! Must be convertible from all sorts of stuff
struct MOOSE_TOOLS_API error_argument : error_argument_type {

	public:
		MOOSE_TOOLS_API explicit error_argument(const char *n_string);
		MOOSE_TOOLS_API explicit error_argument(const std::string &n_string);
		MOOSE_TOOLS_API explicit error_argument(const boost::any &n_something);
		MOOSE_TOOLS_API virtual ~error_argument() noexcept;
};

} // namespace tools
} // namespace moose

