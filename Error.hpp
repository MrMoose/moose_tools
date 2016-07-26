
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MooseToolsConfig.hpp"

#include <boost/exception/all.hpp>
#include <boost/any.hpp>
#include <boost/cstdint.hpp>

#include <string>
#include <sstream>

namespace moose {
namespace tools {

MOOSE_TOOLS_API std::string get_last_error();
MOOSE_TOOLS_API void set_last_error(const std::string& n_error_message);
	
struct MOOSE_TOOLS_API moose_error: virtual std::exception, virtual boost::exception {

	public:
		MOOSE_TOOLS_API virtual char const *what() const noexcept;
		MOOSE_TOOLS_API virtual ~moose_error() noexcept { }
};


//! An exception that is only to be used in unit tests. Allows 
//! for separation of test specific exceptions and 'real' ones
struct MOOSE_TOOLS_API unit_test_error : virtual moose_error {
	
	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! Some component violated protocol specifications and talked BS
struct MOOSE_TOOLS_API protocol_error : virtual moose_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! Unspecified internal fuckup
struct MOOSE_TOOLS_API internal_error : virtual moose_error {
	
	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! UI structuring internal error
struct MOOSE_TOOLS_API ui_error : virtual internal_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! Generic IO error
struct MOOSE_TOOLS_API io_error : virtual moose_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! Serialization failed (not impl specific)
struct MOOSE_TOOLS_API serialization_error : virtual io_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! Disk IO failed
struct MOOSE_TOOLS_API file_error : virtual io_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};

//! Network IO failed
struct MOOSE_TOOLS_API network_error : virtual io_error {

	MOOSE_TOOLS_API virtual char const *what() const noexcept;
};


//! tag any exception with an error code
typedef boost::error_info<struct tag_errno_code, int> errno_code;

//! tag any exception with a human readable error message
typedef boost::error_info<struct tag_error_message, std::string> error_message;

//! tag protocol exceptions with an request / response id
typedef boost::error_info<struct tag_request_id, boost::uint64_t> request_id;

//! tag exceptions with an session id
typedef boost::error_info<struct tag_session_id, boost::uint64_t> session_id;

//! The argument that led to the error
typedef boost::error_info<struct tag_error_argument, std::string> error_argument_type;

//! Must be convertible from all sorts of stuff
struct MOOSE_TOOLS_API error_argument : error_argument_type {

	public:
		MOOSE_TOOLS_API explicit error_argument(const char *n_string);
		MOOSE_TOOLS_API explicit error_argument(const std::string &n_string);
		MOOSE_TOOLS_API explicit error_argument(const boost::any &n_something);
		MOOSE_TOOLS_API virtual ~error_argument(void) noexcept;
};

}
}

