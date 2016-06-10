
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/asio/ip/tcp.hpp>

#include <string>

namespace moose {
namespace tools {

//! truncate a string to at most n characters
void MOOSE_TOOLS_API truncate(std::string &n_string, std::size_t n_length) noexcept;


/*! @brief google uses an endpoint representation string that I don't exactly know but try to parse here

	examples:
	* ipv6:[2a02:810d:e40:67c:adb3:f561:144f:89f5]:61176
	* ipv4:192.168.178.30:61185

	@throw network_error on cannot parse
 */
void MOOSE_TOOLS_API from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address, unsigned short int &n_port);

/*! @brief google uses an endpoint representation string that I don't exactly know but try to parse here

	examples:
	* ipv6:[2a02:810d:e40:67c:adb3:f561:144f:89f5]:61176
	* ipv4:192.168.178.30:61185

	@throw network_error on cannot parse
*/
void MOOSE_TOOLS_API from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address);



/*
  This should really be a template to fit into boost algos

template<typename SequenceT>
void trim(SequenceT &n_string);

}
*/

}
}

