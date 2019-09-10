
//  Copyright 2019 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "Log.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/system/system_error.hpp>
#include <boost/lexical_cast.hpp>

#include <string>



#include <boost/asio/yield.hpp>

// only used by async_timed_connect. Do not use this, look below
template<int Protocol = 4>
struct async_timed_connect_implementation {

	// The implementation holds a pointer to the socket as it is used for
	// multiple async_write operations but owned outside
	boost::asio::ip::tcp::socket      &m_socket;
	boost::asio::ip::tcp::resolver     m_resolver;
	std::unique_ptr<std::string>       m_hostname;
	std::unique_ptr<std::string>       m_portstr;

	// A steady timer used for the timeout check
	std::unique_ptr<boost::asio::steady_timer> m_timeout_timer;
	const unsigned int                 m_timeout;

	// The coroutine state.
	boost::asio::coroutine             m_coro;

	// The first argument to our function object's call operator is a reference
	// to the enclosing intermediate completion handler. This intermediate
	// completion handler is provided for us by the boost::asio::async_compose
	// function, and takes care of all the details required to implement a
	// conforming asynchronous operation. When calling an underlying asynchronous
	// operation, we pass it this enclosing intermediate completion handler
	// as the completion token.
	//
	// All arguments after the first must be defaulted to allow the state machine
	// to be started, as well as to allow the completion handler to match the
	// completion signature of both the async_write and steady_timer::async_wait
	// operations.
	template <typename Self>
	void operator()(Self &n_self, const boost::system::error_code &n_error = boost::system::error_code(),
				boost::asio::ip::tcp::resolver::results_type n_results = boost::asio::ip::tcp::resolver::results_type(),
				boost::asio::ip::tcp::endpoint n_connected_ep = boost::asio::ip::tcp::endpoint()) {

		using boost::asio::ip::tcp;
		using namespace moose::tools;

		// This is the coroutine impl. Basically a long switch clause which jumps 
		// back in where yield was last called .
		// Every time an async ops inside yields, the state is saved for the next call.
		// Error is the global n_error above

		reenter(m_coro) {

			// Do not use switch / break in here...
			if (Protocol == 0) {
				yield m_resolver.async_resolve(*m_hostname, *m_portstr, tcp::resolver::query::numeric_service, std::move(n_self));
			} else if (Protocol == 4) {
				yield m_resolver.async_resolve(tcp::v4(), *m_hostname, *m_portstr, tcp::resolver::query::numeric_service, std::move(n_self));
			} else if (Protocol == 6) {
				yield m_resolver.async_resolve(tcp::v6(), *m_hostname, *m_portstr, tcp::resolver::query::numeric_service, std::move(n_self));
			} else {
				n_self.complete(boost::asio::error::no_protocol_option);
				return;
			}
			
			if (n_error) {
				n_self.complete(n_error);
				return;
			}

			// First set a timeout for the connect operation only. We are 
			// composed but I have to derive from the pattern as I cannot yield after the expires_after
			// and the socket operation is the only one I know how to cancel
			m_timeout_timer->expires_after(std::chrono::seconds(m_timeout));
			m_timeout_timer->async_wait([socket{ &m_socket }](const boost::system::error_code &n_error) {

				if (n_error == boost::asio::error::operation_aborted) {
					return;
				}

				socket->close();
			});
			
			// I need to use the range connect to try all endpoints. In order to do so, I need a different handler signature than
			// the one in here. Apparently I can't have a second handler but I found this in beast which can apparently be used
			// to bind unmatching handlers.
			yield boost::asio::async_connect(m_socket, n_results, 
					boost::beast::bind_handler(std::move(std::forward<Self>(n_self)), boost::placeholders::_1,
							boost::asio::ip::tcp::resolver::results_type(), boost::placeholders::_2));

			m_timeout_timer->cancel();

			if (n_error) {
				if (n_error == boost::asio::error::operation_aborted ) {
					n_self.complete(boost::asio::error::timed_out);
				} else {
					n_self.complete(n_error);
				}

				return;
			}


			// If our socket is closed here, our timeout has killed the operation before we ran
			if (!m_socket.is_open()) {
				n_self.complete(boost::asio::error::timed_out);			
				return;
			}

			boost::system::error_code ignored;
			tcp::endpoint remote_ep{ m_socket.remote_endpoint(ignored) };
			BOOST_LOG_SEV(logger(), debug) << "Socket " << &m_socket << " connected to " << remote_ep;

			m_hostname.reset();
			m_portstr.reset();

			// Call the user-supplied handler with the result of the operation.
			n_self.complete(n_error);
		}
	}
};

#include <boost/asio/unyield.hpp>



namespace moose {
namespace tools {

/*! @brief a composed resolve and connect operation with a timeout.
	
	You may specify a filter to use only IPv4 or v6 by using the template parameter Protocol.
	If the timeout hits, the socket will be closed after the operation and the completion token
	is called with the error code boost::asio::error::timed_out

	@param n_socket will be implicitly opened and connected. Closed on error
	@param n_hostname should resolve to whatever you want
	@param n_port specify where you want to connect to
	@param n_timeout for the connect operation in seconds. I assume the resolve to timeout by itself
	@param n_token a completion handler with signature void (boost::system::error_code). Error will be set on error.
 */
template <int Protocol = 0, typename CompletionToken>
auto async_timed_connect(boost::asio::ip::tcp::socket &n_socket,
			const std::string &n_hostname, const boost::uint16_t n_port, const unsigned int n_timeout,
			CompletionToken &&n_token)

	// The return type of the initiating function is deduced from the combination
	// of CompletionToken type and the completion handler's signature. When the
	// completion token is a simple callback, the return type is always void.
	// In this example, when the completion token is boost::asio::yield_context
	// (used for stackful coroutines) the return type would be also be void, as
	// there is no non-error argument to the completion handler. When the
	// completion token is boost::asio::use_future it would be std::future<void>.
	-> typename boost::asio::async_result<
			typename std::decay<CompletionToken>::type,
			void (boost::system::error_code)
	>::return_type {

	// resolver will get moved to the impl
	boost::asio::ip::tcp::resolver resolver(n_socket.get_executor());

	std::unique_ptr<std::string> hostname{ new std::string(n_hostname) };
	std::unique_ptr<std::string> portstr{ new std::string(boost::lexical_cast<std::string>(n_port)) };

	// Create a steady_timer to be used for the timeout
	std::unique_ptr<boost::asio::steady_timer> timeout_timer{ new boost::asio::steady_timer(n_socket.get_executor()) };

	// The boost::asio::async_compose function takes:
	//
	// - our asynchronous operation implementation,
	// - the completion token,
	// - the completion handler signature, and
	// - any I/O objects (or executors) used by the operation
	//
	// It then wraps our implementation in an intermediate completion handler
	// that meets the requirements of a conforming asynchronous operation. This
	// includes tracking outstanding work against the I/O executors associated
	// with the operation (in this example, this is the socket's executor).
	return boost::asio::async_compose<CompletionToken, void (boost::system::error_code)>(
		async_timed_connect_implementation<Protocol>{
				std::ref(n_socket),
				std::move(resolver),
				std::move(hostname),
				std::move(portstr),
				std::move(timeout_timer),
				n_timeout,
				boost::asio::coroutine() },
		n_token, n_socket);
}


#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void ConnectorgetRidOfLNK4221();
#endif

}
}

