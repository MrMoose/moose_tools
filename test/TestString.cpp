
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE StringTests
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include "../String.hpp"
#include "../Error.hpp"
#include "../Random.hpp"

#include <set>
#include <iostream>

using namespace moose::tools;

BOOST_AUTO_TEST_CASE(TruncateSimple) {
	
	std::string s("Moose was here");

	BOOST_REQUIRE_NO_THROW(truncate(s, 5));
	BOOST_CHECK(s == "Moose");
	BOOST_REQUIRE_NO_THROW(truncate(s, 15));
	BOOST_CHECK(s == "Moose");
	BOOST_REQUIRE_NO_THROW(truncate(s, 1));
	BOOST_CHECK(s == "M");
	BOOST_REQUIRE_NO_THROW(truncate(s, 0));
	BOOST_CHECK(s.empty());
}

BOOST_AUTO_TEST_CASE(ParseIp) {

	boost::asio::ip::address addr;
	unsigned short int port;

	BOOST_CHECK_NO_THROW(from_google_ep("ipv4:192.168.178.30:61184", addr, port));
	BOOST_CHECK_NO_THROW(from_google_ep("ipv6:[2a02:810d:e40:67c:adb3:f561:144f:89f5]:61176", addr, port));
	BOOST_CHECK_THROW(from_google_ep("ipv6:[2a02:810dsgdc:adb3:f561:144f:89f5]:61176", addr, port), network_error);
	BOOST_CHECK_THROW(from_google_ep("ipdyfa6", addr, port), network_error);
	BOOST_CHECK_THROW(from_google_ep("ipv4:192.", addr, port), network_error);
}

BOOST_AUTO_TEST_CASE(MimeTypeGuess) {

	BOOST_CHECK(boost::algorithm::equals(mime_extension("default"), "application/text"));
	BOOST_CHECK(boost::algorithm::equals(mime_extension("test.html"), "text/html"));
	BOOST_CHECK(boost::algorithm::equals(mime_extension("/longer/path/test.html"), "text/html"));
	BOOST_CHECK(boost::algorithm::equals(mime_extension("http://www.test.de/longer/path/test.ico"), "image/vnd.microsoft.icon"));
}
