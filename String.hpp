
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/filesystem/path.hpp>

#include <string>

namespace moose {
namespace tools {

//! truncate a string to at most n characters
void MOOSE_TOOLS_API truncate(std::string &n_string, std::size_t n_length) noexcept;

/*
This should really be a template to fit into boost algos

template<typename SequenceT>
void trim(SequenceT &n_string);

}
*/

/*! @brief google uses an endpoint representation string that I don't exactly know but try to parse here

	examples:
	* ipv6:[2a02:810d:e40:67c:adb3:f561:144f:89f5]:61176
	* ipv4:192.168.178.30:61185

	@throw network_error on cannot parse
 */
MOOSE_TOOLS_API void from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address, unsigned short int &n_port);

/*! @brief google uses an endpoint representation string that I don't exactly know but try to parse here

	examples:
	* ipv6:[2a02:810d:e40:67c:adb3:f561:144f:89f5]:61176
	* ipv4:192.168.178.30:61185

	@throw network_error on cannot parse
*/
MOOSE_TOOLS_API void from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address);

MOOSE_TOOLS_API std::string endpoint_to_string(const boost::asio::ip::udp::endpoint &n_endpoint);

MOOSE_TOOLS_API boost::asio::ip::udp::endpoint string_to_udp_endpoint(const std::string &n_endpoint);

//! spirit itoa wrapper. Always succeeds except bad_alloc
MOOSE_TOOLS_API std::string itoa(const boost::uint64_t n_number);


/*! guess a http mime type from a file extension
	defaults to "application/text" for unrecognized endings
*/
MOOSE_TOOLS_API const char *mime_extension(const std::string &n_path);

/*! guess a http mime type from a file extension
	defaults to "application/text" for unrecognized endings
*/
MOOSE_TOOLS_API const char *mime_extension_from_path(const boost::filesystem::path &n_path);

//! returns a reverse of a string
MOOSE_TOOLS_API std::string reverse(const std::string &n_string);

//! @return true if argument starts with text/
MOOSE_TOOLS_API bool mime_type_is_text(const char *n_string);


}
}

