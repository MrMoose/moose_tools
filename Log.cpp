
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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/gregorian/gregorian_io.hpp>

#include <string>
#include <iostream>
#include <fstream>

namespace moose {
namespace tools {

boost::filesystem::path s_logfile_name;

namespace fs = boost::filesystem;

// Declare attribute keywords
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

static const char* const severity_strings[] = {

	"<DEBUG> ",
	"<INFO> ",
	"<WARNING> ",
	"<ERROR> ",
	"<CRITICAL> "
};

template< typename CharT, typename TraitsT >
std::basic_ostream< CharT, TraitsT > &operator<<(std::basic_ostream< CharT, TraitsT > &n_os, severity_level n_level) {
	
	const char* str = severity_strings[n_level];
	if (n_level < 5 && n_level >= 0)
		n_os << str;
	else
		n_os << static_cast< int >(n_level);
	return n_os;
}

void prepare_log_file() {

	using namespace boost::posix_time;

#ifdef MOOSE_TOOLS_LOG_FILE_DIR
	s_logfile_name = fs::path(MOOSE_TOOLS_LOG_FILE_DIR);
	boost::system::error_code ignored;
	fs::create_directories(s_logfile_name, ignored);
#endif

#ifdef MOOSE_TOOLS_LOG_FILE_NAME
	s_logfile_name /= MOOSE_TOOLS_LOG_FILE_NAME;
#else
	s_logfile_name /= "default.log";
#endif
		
	// If the log file already exists, try to rotate it away and append the current time as string
	if (fs::exists(s_logfile_name)) {
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		std::stringstream ss_date;
		std::stringstream ss_time;

		boost::gregorian::date_facet* date_facet = new boost::gregorian::date_facet();
		date_facet->format("%Y-%m-%d_");
		ss_date.imbue(std::locale(std::locale::classic(), date_facet));

		boost::posix_time::time_facet* time_facet = new boost::posix_time::time_facet();
		time_facet->format("%H-%M-%S");
		ss_time.imbue(std::locale(std::locale::classic(), time_facet));

		ss_date << now.date();
		ss_time << now;
		
		fs::path backup_location(s_logfile_name);
		backup_location.replace_extension("log_" + ss_date.str() + ss_time.str());
	
		fs::copy(s_logfile_name, backup_location, ignored);
	}
}


#ifdef _WIN32

void init_logging(void) {
	
	namespace logging = boost::log;
	namespace sinks = boost::log::sinks;
	namespace expr = boost::log::expressions;

	logging::formatter fmt = expr::stream
		<< expr::attr< unsigned int >("LineID") << ": "
		<< "[" << timestamp << "] "
		<< severity
		<< expr::smessage;

#ifdef MOOSE_TOOLS_FILE_LOG

	prepare_log_file();

	boost::shared_ptr<FileSink> file_sink = boost::make_shared<FileSink>();

	// Add a stream to write log to
	boost::shared_ptr<std::ofstream> ofstr = boost::make_shared<boost::filesystem::ofstream>(s_logfile_name);
	file_sink->locked_backend()->add_stream(ofstr);
	file_sink->locked_backend()->auto_flush(true);
	file_sink->set_formatter(fmt);
#ifndef MOOSE_DEBUG
	file_sink->set_filter(severity >= severity_level::normal);
#endif // MOOSE_DEBUG
	logging::core::get()->add_sink(file_sink);
#endif // MOOSE_TOOLS_FILE_LOG

#ifdef MOOSE_TOOLS_CONSOLE_LOG

	// We have to provide an empty deleter to avoid destroying the global stream object
	boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
	boost::shared_ptr<ConsoleSink> console_sink = boost::make_shared<ConsoleSink>();
	console_sink->locked_backend()->add_stream(stream);
	console_sink->set_formatter(fmt);
#ifndef MOOSE_DEBUG
	console_sink->set_filter(severity >= severity_level::warning);
#endif // MOOSE_DEBUG
	logging::core::get()->add_sink(console_sink);
#endif

#ifdef MOOSE_TOOLS_EVENT_LOG
	// Create an event log sink
	boost::shared_ptr<EventSink> event_sink = boost::make_shared<EventSink>();

	// We'll have to map our custom levels to the event log event types
	sinks::event_log::custom_event_type_mapping< severity_level > mapping("Severity");
	mapping[debug] = sinks::event_log::success;
	mapping[normal] = sinks::event_log::info;
	mapping[warning] = sinks::event_log::warning;
	mapping[error] = sinks::event_log::error;
	event_sink->locked_backend()->set_event_type_mapper(mapping);
#ifndef MOOSE_DEBUG
	event_sink->set_filter(severity >= severity_level::warning);
#endif // MOOSE_DEBUG
	logging::core::get()->add_sink(event_sink);
#endif

	logging::add_common_attributes();
	logging::core::get()->add_global_attribute("Scope", boost::log::attributes::named_scope());
}





#else  // Linux init_logging()






void init_logging(void) {
	
	namespace logging = boost::log;
	namespace sinks = boost::log::sinks;
	namespace expr = boost::log::expressions;

	logging::formatter fmt = expr::stream
		<< expr::attr< unsigned int >("LineID") << ": "
		<< "[" << timestamp << "] "
		<< severity
		<< expr::smessage;


	// Create an event log sink
    boost::shared_ptr<DefaultSink> sink = boost::make_shared<DefaultSink>(
                logging::keywords::facility = sinks::syslog::user);

#ifdef MOOSE_TOOLS_FILE_LOG

	prepare_log_file();

	boost::shared_ptr<FileSink> file_sink = boost::make_shared<FileSink>();

	// Add a stream to write log to
	boost::shared_ptr<std::ofstream> ofstr = boost::make_shared<boost::filesystem::ofstream>(s_logfile_name);

	file_sink->locked_backend()->add_stream(ofstr);
	file_sink->locked_backend()->auto_flush(true);
	file_sink->set_formatter(fmt);
#ifndef MOOSE_DEBUG
	file_sink->set_filter(severity >= severity_level::normal);
#endif // MOOSE_DEBUG
	logging::core::get()->add_sink(file_sink);
#endif

#ifdef MOOSE_TOOLS_CONSOLE_LOG
	// We have to provide an empty deleter to avoid destroying the global stream object
	boost::shared_ptr<std::ostream> stream(&std::clog, boost::null_deleter());
	boost::shared_ptr<ConsoleSink> console_sink = boost::make_shared<ConsoleSink>();
	console_sink->locked_backend()->add_stream(stream);

	console_sink->set_formatter(fmt);
#ifndef MOOSE_DEBUG
	console_sink->set_filter(severity >= severity_level::warning);
#endif // MOOSE_DEBUG
	logging::core::get()->add_sink(console_sink);
#endif

    sink->locked_backend()->set_severity_mapper(sinks::syslog::direct_severity_mapping<int>("Severity"));

	// Add the sink to the core
	logging::core::get()->add_sink(sink);

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

