
//  Copyright 2018 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/asio/streambuf.hpp>
#include <boost/iostreams/concepts.hpp>

namespace moose {
namespace tools {

/*! @brief a model of the Boost.IOStream's Source concept
	for reading data from a Boost.Asio streambuf with the ability to limit extracted bytes
	Found on https://stackoverflow.com/questions/28415142/how-to-deal-with-extra-characters-read-into-asio-streambuf
	Courtesy of Tanner Sansbury.

	Again, I did not write this. Tanner Sansbury did.

	Usage:
	boost::asio::streambuf sbuf;
	...
	boost::iostreams::stream<moose::tools::asio_streambuf_input_device> os(sbuf, <limit>);

	...reading from stream now is limited to <limit> characters
 */
class asio_streambuf_input_device : public boost::iostreams::source {

	public:
		//! @brief construct with limit as amount of characters to read
		MOOSE_TOOLS_API asio_streambuf_input_device(boost::asio::streambuf &n_streambuf, const std::streamsize n_limit);

		MOOSE_TOOLS_API std::streamsize read(char_type * const n_buffer, const std::streamsize n_buffer_size);

	private:
		boost::asio::streambuf &m_streambuf;
		std::streamsize         m_bytes_remaining;
};

}
}

