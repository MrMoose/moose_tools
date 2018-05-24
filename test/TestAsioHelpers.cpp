
//  Copyright 2018 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE AsioHelpersTests
#include <boost/test/unit_test.hpp>

#include "../AsioHelpers.hpp"

#include <boost/iostreams/stream.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

namespace asio      = boost::asio;
namespace iostreams = boost::iostreams;

using moose::tools::asio_streambuf_input_device;

BOOST_AUTO_TEST_CASE(CheckStreambufLimit) {

	char test[128];
	memset(test, 0, 128);
	asio::streambuf sbuf;

	{
		// fill streambuf with some characters
		std::iostream sin(&sbuf);
		sin << "Hello World!";
	}

	{
		// construct limited stream to extract from
		iostreams::stream<asio_streambuf_input_device> os(sbuf, 5);

		// I expect this to read only 5 because limit
		os.read(test, 127);

		BOOST_CHECK(boost::algorithm::equals(test, "Hello"));
	}

	{
		// put more characters 
		std::iostream sin(&sbuf);
		sin << " This is a Test";
	}

	{
		// now for a second time
		iostreams::stream<asio_streambuf_input_device> os(sbuf, 5);
		BOOST_CHECK(os.good());
		BOOST_CHECK(!os.eof());
		BOOST_CHECK(!os.bad());
		BOOST_CHECK(!os.fail());

		// I expect this to read only 5 because limit
		os.read(test, 127);

		BOOST_CHECK(boost::algorithm::equals(test, " Worl"));
		BOOST_CHECK(os.eof());
		BOOST_CHECK(!os.bad());
		BOOST_CHECK(!os.fail());
	}
}
