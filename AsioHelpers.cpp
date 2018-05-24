
//  Copyright 2018 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "AsioHelpers.hpp"

#include <boost/asio.hpp>

namespace moose {
namespace tools {

asio_streambuf_input_device::asio_streambuf_input_device(boost::asio::streambuf &n_streambuf, const std::streamsize n_limit)
		: m_streambuf(n_streambuf)
		, m_bytes_remaining(n_limit) {

}

std::streamsize asio_streambuf_input_device::read(char_type *const n_buffer, const std::streamsize n_buffer_size) {

	// Determine max amount of bytes to copy.
	const std::streamsize bytes_to_copy =
		std::min(m_bytes_remaining, std::min(
			static_cast<std::streamsize>(m_streambuf.size()), n_buffer_size));

	// If there is no more data to be read, indicate end-of-sequence per
	// Source concept.
	if (0 == bytes_to_copy) {
		return -1; // Indicate end-of-sequence, per Source concept.
	}

	// Copy from the streambuf into the provided buffer.
	std::copy_n(boost::asio::buffers_begin(m_streambuf.data()), bytes_to_copy, n_buffer);

	// Update bytes remaining.
	m_bytes_remaining -= bytes_to_copy;

	// Consume from the streambuf.
	m_streambuf.consume(bytes_to_copy);

	return bytes_to_copy;
}

}
}
