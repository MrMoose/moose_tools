
//  Copyright 2019 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/system_error.hpp>


#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>

#include <string>





#include <boost/asio/yield.hpp>

// only used by async_timed_connect. Do not use this, look below
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
				boost::asio::ip::tcp::resolver::results_type n_results = boost::asio::ip::tcp::resolver::results_type()) {

		using namespace boost::asio::ip;

		// This is the coroutine impl. Basically a long switch clause which jumps 
		// back in where yield was last called .
		// Every time an async ops inside yields, the state is saved for the next call.
		// Error is the global n_error above

		reenter(m_coro) {

			yield m_resolver.async_resolve(*m_hostname, *m_portstr, tcp::resolver::query::numeric_service, std::move(n_self));
			if (n_error) {
				std::cerr << "resolve error " << n_error << std::endl;
				n_self.complete(n_error);
				return;
			}

			// First set a timeout for the connect operation only. We are 
			// composed but I have to derive from the pattern as I cannot yield after the expires_after
			// and the socket operation is the only one I know how to cancel
			m_timeout_timer->expires_after(std::chrono::seconds(m_timeout));
			m_timeout_timer->async_wait([expiry{ m_timeout_timer->expiry() }, socket{ &m_socket }](const boost::system::error_code &n_error) {

				if (n_error == boost::asio::error::operation_aborted) {
					std::cout << "timer aborted" << std::endl;
					return;
				}

				// Check whether the deadline has passed. We compare the deadline against
				// the current time since a new asynchronous operation may have moved the
				// deadline before this actor had a chance to run.
				if (expiry <= boost::asio::steady_timer::clock_type::now()) {
					// The deadline has passed. The socket is closed so that any outstanding
					// asynchronous operations are cancelled. This means the async_connect operation,
					// which will also answer the callback completion handler.
					// As we know the coroutine has not continued past the connect operation,
					// we may access the ptr in this case
					socket->close();
				}
			});
			
			yield m_socket.async_connect(*n_results, std::move(n_self));
			if (n_error) {
				if (n_error == boost::asio::error::operation_aborted ) {
					n_self.complete(boost::asio::error::timed_out);
				} else {
					n_self.complete(n_error);
				}

				return;
			}

			m_timeout_timer->cancel();

			// If our socket is closed here, our timeout has killed the operation before we ran
			if (!m_socket.is_open()) {
				n_self.complete(boost::asio::error::timed_out);			
				return;
			}

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




template <typename CompletionToken>
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

	using namespace boost::asio::ip;


	tcp::resolver resolver(n_socket.get_executor());

	std::unique_ptr<std::string> hostname{ new std::string(n_hostname) };
	std::unique_ptr<std::string> portstr{ new std::string(boost::lexical_cast<std::string>(n_port)) };

	// Create a steady_timer to be used for the timeout
	std::unique_ptr<boost::asio::steady_timer> timeout_timer{ new boost::asio::steady_timer(n_socket.get_executor()) };



// 	std::shared_ptr<async_timed_connect_implementation> impl{ new async_timed_connect_implementation{
// 		std::ref(n_socket),
// 		std::move(resolver),
// 		std::move(hostname),
// 		std::move(portstr),
// 		std::move(timeout_timer),
// 		n_timeout,
// 		boost::asio::coroutine()
// 	} };




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
		async_timed_connect_implementation{
				std::ref(n_socket),
				std::move(resolver),
				std::move(hostname),
				std::move(portstr),
				std::move(timeout_timer),
				n_timeout,
				boost::asio::coroutine() },
		n_token, n_socket);
}















/*! @brief connect a stream socket with a timeout.
	Caller must ensure that n_socket lives longer than the lifetime
	of this operation
	


namespace {

	template<typename CompletionToken>
	void do_connect(boost::asio::ip::tcp::socket &n_socket, const boost::asio::ip::tcp::resolver::results_type &n_endpoints, unsigned int n_timeout, CompletionToken &&n_callback) noexcept {

		// Timer will share ownership between the actual connect handler and the timeout handler.
		// Whichever lives longer will destroy the timer.
		std::shared_ptr<boost::asio::steady_timer> timer(std::make_shared<boost::asio::steady_timer>(n_socket.get_executor()));

		// Set a deadline for the connect operation.
		timer->expires_after(std::chrono::seconds(n_timeout));

		// Start the timer which will react on the deadline passing
		timer->async_wait([timer, &n_socket](const boost::system::error_code &n_error) {

			if (n_error == boost::asio::error::operation_aborted) {
				// Connection timeout cancelled. Normal case
				return;
			}

			// Check whether the deadline has passed. We compare the deadline against
			// the current time since a new asynchronous operation may have moved the
			// deadline before this actor had a chance to run.
			if (timer->expiry() <= boost::asio::steady_timer::clock_type::now()) {
				// The deadline has passed. The socket is closed so that any outstanding
				// asynchronous operations are cancelled. This means the async_connect operation,
				// which will also answer the callback completion handler
				n_socket.close();
			}
		});

		boost::asio::async_connect(n_socket, n_endpoints,
			[comp_token{ std::move(n_callback) }, timer, &n_socket](const boost::system::error_code &n_errc, const boost::asio::ip::tcp::endpoint &n_endpoint) {

			// The async_connect() function automatically opens the socket at the start
			// of the asynchronous operation. If the socket is closed at this time then
			// the timeout handler must have run first. We answer the token and are done
			if (!n_socket.is_open()) {
				comp_token(boost::asio::error::timed_out);
				return;
			}

			// Otherwise cancel the timer. It will abort itself next chance
			timer->cancel();

			// The caller may take care of whatever else may have happened. We may be connected, we may not.
			// we do expect however for the timeout to not have passed
			comp_token(n_errc);
			return;
		});
	}
} // anon ns








/*! @brief initiate connection to 

	Callback must be void (boost::system::error_code) noexcept or something along those lines.

	I'm assuming 

 

template<typename CompletionToken>
void async_timed_connect(boost::asio::ip::tcp::socket &n_socket, const std::string &n_hostname, const boost::uint16_t n_port, CompletionToken &&n_callback, unsigned int n_timeout = 8) noexcept {
	
	using namespace boost::asio::ip;

	tcp::resolver resolver(n_socket.get_executor());
	tcp::resolver::query query(n_hostname, boost::lexical_cast<std::string>(n_port), tcp::resolver::query::numeric_service);
	
	// I don't set the timeout here already as I expect the resolver to have its own.
	resolver.async_resolve(query,
		[comp_token{ std::move(n_callback) }, &n_socket, n_timeout]
				(const boost::system::error_code &n_err, const tcp::resolver::results_type &n_endpoints) {

			if (n_err) {
				comp_token(n_err);
				return;
			}

			// If we haven't found any endpoints but there's no error it means it cannot be resolved.
			if (n_endpoints.empty()) {				
				comp_token(boost::asio::error::host_not_found);
				return;
			}

			do_connect(n_socket, n_endpoints, n_timeout, comp_token);
	});
}
*/
#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void ConnectorgetRidOfLNK4221();
#endif

}
}

