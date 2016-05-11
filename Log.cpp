
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Log.hpp"

#ifdef _WIN32
#include <boost/log/sinks/event_log_backend.hpp>
#else
#endif
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <string>
#include <iostream>
#include <fstream>

namespace moose {
namespace tools {

// Declare attribute keywords
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)


#ifdef _WIN32

void init_logging(void) {
	
	namespace logging = boost::log;
	namespace sinks = boost::log::sinks;
	namespace expr = boost::log::expressions;

	// Create an event log sink
	boost::shared_ptr<DefaultSink> sink = boost::make_shared<DefaultSink>();

#ifndef NDEBUG
	// We have to provide an empty deleter to avoid destroying the global stream object
	boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
	boost::shared_ptr<ConsoleSink> csink = boost::make_shared<ConsoleSink>();
	csink->locked_backend()->add_stream(stream);
#endif

#ifndef MOOSE_TOOLS_EVENT_LOG
	// Add a stream to write log to
	boost::shared_ptr<std::ofstream> ofstr = boost::make_shared<std::ofstream>("default.log");
	sink->locked_backend()->add_stream(ofstr);
#endif

	sink->set_formatter(
		expr::stream
			<< expr::attr< unsigned int >("LineID") << ": "
			<< expr::smessage
			<< " (" << expr::attr< std::string >("Scope") << ")"
		);

#ifdef MOOSE_TOOLS_EVENT_LOG
	// We'll have to map our custom levels to the event log event types
	sinks::event_log::custom_event_type_mapping< severity_level > mapping("Severity");
	mapping[debug] = sinks::event_log::success;
	mapping[normal] = sinks::event_log::info;
	mapping[warning] = sinks::event_log::warning;
	mapping[error] = sinks::event_log::error;
	sink->locked_backend()->set_event_type_mapper(mapping);
#endif

	// Add the sink to the core
	logging::core::get()->add_sink(sink);

#ifndef NDEBUG
	logging::core::get()->add_sink(csink);
#endif

	logging::add_common_attributes();
	logging::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());
}

#else  // Linux init_logging()

void init_logging(void) {
	
	namespace logging = boost::log;
	namespace sinks = boost::log::sinks;
	namespace expr = boost::log::expressions;

	// Create an event log sink
    boost::shared_ptr<DefaultSink> sink = boost::make_shared<DefaultSink>(
                logging::keywords::facility = sinks::syslog::user);

#ifndef NDEBUG
	// We have to provide an empty deleter to avoid destroying the global stream object
	boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
	boost::shared_ptr<ConsoleSink> csink = boost::make_shared<ConsoleSink>();
	csink->locked_backend()->add_stream(stream);
#endif

	sink->set_formatter(
		expr::stream
			<< expr::attr< unsigned int >("LineID") << ": "
			<< expr::smessage
			<< " (" << expr::attr< std::string >("Scope") << ")"
		);

    sink->locked_backend()->set_severity_mapper(sinks::syslog::direct_severity_mapping<int>("Severity"));

	// Add the sink to the core
	logging::core::get()->add_sink(sink);

#ifndef NDEBUG
	logging::core::get()->add_sink(csink);
#endif

	logging::add_common_attributes();
	logging::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());
}

#endif


	BOOST_LOG_GLOBAL_LOGGER_INIT(s_moose_logger, DefaultLogger) {

	boost::log::sources::severity_logger_mt< severity_level > lg;
	return lg;
}


DefaultLogger &logger(void) noexcept {

	return s_moose_logger::get();
}

}
}

