
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MooseToolsConfig.hpp"

#include <boost/log/trivial.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

#ifdef _WIN32
#include <boost/log/sinks/event_log_backend.hpp>
#else
#include <boost/log/sinks/syslog_backend.hpp>
#endif
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace moose {
namespace tools {

// Define application-specific severity levels
enum severity_level {
	debug,
	normal,
	warning,
	error,
	critical
};

#ifdef _WIN32

#ifdef MOOSE_TOOLS_EVENT_LOG
	// Complete sink type
	typedef boost::log::sinks::synchronous_sink< boost::log::sinks::simple_event_log_backend > DefaultSink;
#else
	// log to file instead when insufficient rights
	typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > DefaultSink;
#endif

#else
	// Complete sink type
	typedef boost::log::sinks::synchronous_sink< boost::log::sinks::syslog_backend > DefaultSink;

#endif

typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > ConsoleSink;
typedef boost::log::sources::severity_logger_mt< severity_level > DefaultLogger;

/*! \brief initialize boost logging
	You MUST call this at startup of any logging program. Otherwise your log output may look crappy.
*/
MOOSE_TOOLS_API void init_logging(void);

BOOST_LOG_GLOBAL_LOGGER(s_moose_logger, DefaultLogger)

MOOSE_TOOLS_API DefaultLogger &logger(void) noexcept;


}
}

