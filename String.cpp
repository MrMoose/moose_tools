
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
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <map>

namespace moose {
namespace tools {

namespace fs = boost::filesystem;

void truncate(std::string &n_string, std::size_t n_length) noexcept {

	if (n_string.length() > n_length) {
		n_string.resize(n_length);
	}
}

struct ip4_from_str_impl {

	typedef boost::asio::ip::address_v4  result_type;

	template <typename Arg>
	result_type operator()(const Arg n_str) const {

#if BOOST_VERSION / 100000 >= 1 && BOOST_VERSION / 100 % 1000 >= 66
		return boost::asio::ip::make_address_v4(n_str);
#else
		return boost::asio::ip::address_v4::from_string(n_str);
#endif

	}
};

struct ip6_from_str_impl {

	typedef boost::asio::ip::address_v6  result_type;

	template <typename Arg>
	result_type operator()(const Arg n_str) const {
#if BOOST_VERSION / 100000 >= 1 && BOOST_VERSION / 100 % 1000 >= 66
		return  boost::asio::ip::make_address_v6(n_str);
#else
		return boost::asio::ip::address_v6::from_string(n_str);
#endif
	}
};

namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;
namespace ascii = qi::ascii;
namespace ip = boost::asio::ip;
namespace phx = boost::phoenix;

typedef boost::fusion::vector<ip::address, unsigned int>  ip_tuple;

/*! @brief parse a google format ip and port */
template <typename Iterator>
struct google_ip_parser : qi::grammar<Iterator, ip_tuple()> {

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
	qi::rule<Iterator, ip_tuple()>          m_start;
};

void from_google_ep(const std::string &n_google_ep, boost::asio::ip::address &n_address, unsigned short int &n_port) {
	
	google_ip_parser<std::string::const_iterator> p;
	ip_tuple                           result;
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

std::string endpoint_to_string(const boost::asio::ip::udp::endpoint &n_endpoint) {

	std::ostringstream oss;
	oss << n_endpoint;
	return oss.str();
}


/*! @brief parse a normal format ip and port */
template <typename Iterator>
struct ip_parser : qi::grammar<Iterator, ip_tuple()> {

	ip_parser() : ip_parser::base_type(m_start, "ip_endpoint") {

		using qi::_1;
		using qi::_2;
		using qi::_val;

		m_ipv4_string %= +qi::char_("0-9\\.");
		m_ipv6_string %= "[" >> +qi::char_(":A-Fa-f0-9\\.") >> "]";

		m_ipv4_address = m_ipv4_string[_val = v4_from_str(_1)];
		m_ipv6_address = m_ipv6_string[_val = v6_from_str(_1)];

		m_start %= (m_ipv4_address | m_ipv6_address) >> ":" >> qi::uint_;
	}

	boost::phoenix::function<ip4_from_str_impl>    v4_from_str;
	boost::phoenix::function<ip6_from_str_impl>    v6_from_str;

	qi::rule<Iterator, std::string()>        m_ipv4_string;
	qi::rule<Iterator, ip::address_v4()>     m_ipv4_address;
	qi::rule<Iterator, std::string()>        m_ipv6_string;
	qi::rule<Iterator, ip::address_v6()>     m_ipv6_address;
	qi::rule<Iterator, ip_tuple()>           m_start;
};

boost::asio::ip::udp::endpoint string_to_udp_endpoint(const std::string &n_endpoint) {

	boost::asio::ip::address address;
	unsigned short int       port = 0;
	ip_parser<std::string::const_iterator> p;
	ip_tuple                               result;
	std::string::const_iterator            begin = n_endpoint.begin();
	const std::string::const_iterator      end = n_endpoint.end();

	try {
		if (qi::parse(begin, end, p, result) && (begin == end)) {
			address = boost::fusion::at_c<0>(result);
			port = boost::fusion::at_c<1>(result);
			return boost::asio::ip::udp::endpoint(address, port);
		} else {
			BOOST_THROW_EXCEPTION(network_error() << error_message("Cannot parse IP") << error_argument(n_endpoint));
		}
	} catch (const std::exception &) {  // asio throws on invalid parse
		BOOST_THROW_EXCEPTION(network_error() << error_message("Cannot parse IP") << error_argument(n_endpoint));
	}
}

std::string itoa(const boost::uint64_t n_number) {

	std::string ret;
	std::back_insert_iterator<std::string> sink(ret);
	karma::generate(sink, karma::ulong_long, n_number);   // why should this fail?
	return ret;
}

// returns a reverse of a string
std::string reverse(const std::string &n_string) {

	return{ n_string.rbegin(), n_string.rend() };
}

struct reverse_mimetype_symbols_type : qi::symbols<char, const char *> {

	reverse_mimetype_symbols_type() {

		this->add
			(reverse(".htm"), "text/html")
			(reverse(".html"), "text/html")
			(reverse(".php"), "text/html")
			(reverse(".css"), "text/css")
			(reverse(".txt"), "text/plain")
			(reverse(".js"), "application/javascript")
			(reverse(".json"), "application/json")
			(reverse(".xml"), "application/xml")
			(reverse(".swf"), "application/x-shockwave-flash")
			(reverse(".flv"), "video/x-flv")
			(reverse(".png"), "image/png")
			(reverse(".jpe"), "image/jpeg")
			(reverse(".jpeg"), "image/jpeg")
			(reverse(".jpg"), "image/jpeg")
			(reverse(".gif"), "image/gif")
			(reverse(".bmp"), "image/bmp")
			(reverse(".ico"), "image/vnd.microsoft.icon")
			(reverse(".tiff"), "image/tiff")
			(reverse(".tif"), "image/tiff")
			(reverse(".svg"), "image/svg+xml")
			(reverse(".svgz"), "image/svg+xml")
			;
	}
};

template <typename Iterator>
struct reverse_mimetype_matching_parser : qi::grammar<Iterator, const char *()> {

	reverse_mimetype_matching_parser() : reverse_mimetype_matching_parser::base_type(m_start, "reverse_mimetype_matching_parser") {

		m_start %= qi::no_case[m_mimetype_symbols] >> qi::eoi;
	}

	reverse_mimetype_symbols_type      m_mimetype_symbols;
	qi::rule<Iterator, const char *()> m_start;
};

// a static instance of the parser can and should be used to avoid creating it all the time
static reverse_mimetype_matching_parser<std::string::const_reverse_iterator> const reverse_mimetype_matching_parser_instance;

const char *mime_extension(const std::string &n_path) {

	const char *                                ret = "application/text";
	std::string::const_reverse_iterator         begin = n_path.crbegin();
	const std::string::const_reverse_iterator   end = n_path.crend();
	qi::parse(begin, end, reverse_mimetype_matching_parser_instance, ret);
	return ret;
}


struct mimetype_symbols_type : qi::symbols<char, const char *> {

	mimetype_symbols_type() {

		this->add
			("htm",  "text/html")
			("html", "text/html")
			("php",  "text/html")
			("css",  "text/css")
			("txt",  "text/plain")
			("js",   "application/javascript")
			("json", "application/json")
			("xml",  "application/xml")
			("swf",  "application/x-shockwave-flash")
			("flv",  "video/x-flv")
			("png",  "image/png")
			("jpe",  "image/jpeg")
			("jpeg", "image/jpeg")
			("jpg",  "image/jpeg")
			("gif",  "image/gif")
			("bmp",  "image/bmp")
			("ico",  "image/vnd.microsoft.icon")
			("tiff", "image/tiff")
			("tif",  "image/tiff")
			("svg",  "image/svg+xml")
			("svgz", "image/svg+xml")
			;
	}
};

template <typename Iterator>
struct mimetype_matching_parser : qi::grammar<Iterator, const char *()> {

	mimetype_matching_parser() : mimetype_matching_parser::base_type(m_start, "mimetype_matching_parser") {

		m_start %= qi::no_case[m_mimetype_symbols] >> qi::eoi;
	}

	mimetype_symbols_type              m_mimetype_symbols;
	qi::rule<Iterator, const char *()> m_start;
};

// a static instance of the parser can and should be used to avoid creating it all the time
static mimetype_matching_parser<std::string::const_iterator> const mimetype_matching_parser_instance;

const char *mime_extension_from_path(const boost::filesystem::path &n_path) {

	const char *ret = "application/text";

	const fs::path extension = n_path.extension();   // contains the entire extension including the dot
	if (extension.empty()) {
		return ret;
	}
	
	const std::string                 extstr = extension.string();   
	std::string::const_iterator       begin = extstr.cbegin() + 1;
	const std::string::const_iterator end = extstr.cend();
	qi::parse(begin, end, mimetype_matching_parser_instance, ret);
	return ret;
}

bool mime_type_is_text(const char *n_string) {
	
	if (boost::algorithm::iequals(n_string, "application/text")) {
		return true;
	}

	using namespace boost::xpressive;

	cregex mimetype_text_re = cregex::compile("^text\\/\\w+");
	return regex_match(n_string, mimetype_text_re);
}

}
}