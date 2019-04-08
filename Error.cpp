
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Error.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/home/karma/numeric/real_policies.hpp>

#include <boost/thread/tss.hpp>

namespace moose {
namespace tools {

//----------------------------------------------------------------------------------------------------------------------
namespace {
	boost::thread_specific_ptr<std::string> tls_error_message;

	inline std::string* get_tls_error_message() {
		if (!tls_error_message.get()) {
			tls_error_message.reset(new std::string{});
		}
		return tls_error_message.get();
	}
} // namespace

//----------------------------------------------------------------------------------------------------------------------
std::string get_last_error() {
	return *get_tls_error_message();
}

//----------------------------------------------------------------------------------------------------------------------
void set_last_error(const std::string& n_error_message) {
	*get_tls_error_message() = n_error_message;
}

moose_error::moose_error() {
#if defined(MOOSE_DEBUG)
	// inject stacktrace for every error
	*this << (stacktrace_dump(boost::stacktrace::stacktrace{}));
#endif
}

namespace karma = boost::spirit::karma;
namespace ascii = boost::spirit::ascii;

error_argument::error_argument(const char *n_string)
		: error_argument_type(n_string) {
}

error_argument::error_argument(const std::string &n_string)
		: error_argument_type(n_string) {
}

// define a new real number formatting policy
template <typename Num>
struct scientific_policy : karma::real_policies<Num> {

	// we want the numbers always to be in scientific format
	static int floatfield(Num n) {
		return karma::real_policies<Num>::fmtflags::scientific;
	}

	static unsigned int precision(Num n) {
		return 15;
	};
};

error_argument::error_argument(const boost::any &n_something)
		: error_argument_type("<cannot determine argument type>") {

	if (n_something.type() == typeid(unsigned int)) {
		std::back_insert_iterator<std::string> out(value());
		karma::generate(out, karma::uint_, boost::any_cast<unsigned int>(n_something));
		// don't care if that fails nothing I could do anyway except allocate more mem
	} else if (n_something.type() == typeid(int)) {
		std::back_insert_iterator<std::string> out(value());
		karma::generate(out, karma::int_, boost::any_cast<int>(n_something));
	} else if (n_something.type() == typeid(double)) {
		// define a new generator type based on the new policy
		typedef karma::real_generator<double, scientific_policy<double> > science_type;
		science_type const scientific = science_type();
		std::back_insert_iterator<std::string> out(value());
		karma::generate(out, scientific, boost::any_cast<double>(n_something));
	} else if (n_something.type() == typeid(float)) {
		// define a new generator type based on the new policy
		typedef karma::real_generator<float, scientific_policy<float> > science_type;
		science_type const scientific = science_type();
		std::back_insert_iterator<std::string> out(value());
		karma::generate(out, scientific, boost::any_cast<float>(n_something));
	}
}

error_argument::~error_argument(void) noexcept {

}

char const * moose_error::what() const noexcept {

	return "Generic error";
}

char const * unit_test_error::what() const noexcept {

	return "Unit Test specific error";
}

char const * protocol_error::what() const noexcept {

	return "Protocol implementation error";
}

char const * internal_error::what() const noexcept {

	return "Internal error";
}

char const * ui_error::what() const noexcept {

	return "UI setup error";
}

char const * io_error::what() const noexcept {

	return "Generic I/O error";
}

char const * serialization_error::what() const noexcept {

	return "Serialization error";
}

char const * file_error::what() const noexcept {

	return "Filesystem error";
}

char const * network_error::what() const noexcept {

	return "Network error";
}

}
}
