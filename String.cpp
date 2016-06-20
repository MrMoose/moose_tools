
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "String.hpp"
#include "Error.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/asio.hpp>

namespace moose {
namespace tools {

void truncate(std::string &n_string, std::size_t n_length) noexcept {

	if (n_string.length() > n_length) {
		n_string.resize(n_length);
	}
}

struct ip4_from_str_impl {

	typedef boost::asio::ip::address_v4  result_type;

	template <typename Arg>
	result_type operator()(const Arg n_str) const {
		return boost::asio::ip::address_v4::from_string(n_str);
	}
};

struct ip6_from_str_impl {

	typedef boost::asio::ip::address_v6  result_type;

	template <typename Arg>
	result_type operator()(const Arg n_str) const {
		return boost::asio::ip::address_v6::from_string(n_str);
	}
};

namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;
namespace ascii = qi::ascii;
namespace ip = boost::asio::ip;
namespace phx = boost::phoenix;

typedef boost::fusion::vector<ip::address, unsigned int>  ip_vector;

/*! \brief parse a google format ip and port */
template <typename Iterator>
struct google_ip_parser : qi::grammar<Iterator, ip_vector()> {

	google_ip_parser() : google_ip_parser::base_type(m_start, "google_ip") {

		using qi::_1;
		using qi::_2;
		using qi::_val;

		m_ipv4_string  %= "ipv4:" >> +qi::char_("0-9\\.");
		m_ipv6_string  %= "ipv6:[" >> +qi::char_(":A-Fa-f0-9\\.") >> "]";

		m_ipv4_address  = m_ipv4_string[_val = v4_from_str(_1)];
		m_ipv6_address  = m_ipv6_string[_val = v6_from_str(_1)];

		m_start        %= (m_ipv4_address | m_ipv6_address) >> ":" >> qi::uint_;
	}

	boost::phoenix::function<ip4_from_str_impl>    v4_from_str;
	boost::phoenix::function<ip6_from_str_impl>    v6_from_str;

	qi::rule<Iterator, std::string()>        m_ipv4_string;
	qi::rule<Iterator, ip::address_v4()>     m_ipv4_address;
	qi::rule<Iterator, std::string()>        m_ipv6_string;
	qi::rule<Iterator, ip::address_v6()>     m_ipv6_address;
	qi::rule<Iterator, ip_vector()>          m_start;
};

void from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address, unsigned short int &n_port) {
	
	google_ip_parser<std::string::const_iterator> p;
	ip_vector                           result;
	std::string::const_iterator         begin = n_google_ep.begin();
	const std::string::const_iterator   end = n_google_ep.end();

	try {
		if (qi::parse(begin, end, p, result) && (begin == end)) {
			n_address = boost::fusion::at_c<0>(result);
			n_port = boost::fusion::at_c<1>(result);
			return;
		} else {
			BOOST_THROW_EXCEPTION(network_error() << error_message("Cannot parse IP") << error_argument(n_google_ep));
		}
	} catch (const std::exception &) {  // asio throws on invalid parse
		BOOST_THROW_EXCEPTION(network_error() << error_message("Cannot parse IP") << error_argument(n_google_ep));
	}
}

void from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address) {

	unsigned short int unused_port = 0;
	return from_google_ep(n_google_ep, n_address, unused_port);
}

std::string itoa(const boost::uint64_t n_number) {

	std::string ret;
	std::back_insert_iterator<std::string> sink(ret);
	karma::generate(sink, karma::uint_, n_number);   // why should this fail?
	return ret;
}

}
}