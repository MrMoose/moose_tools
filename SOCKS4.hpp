
//  Copyright 2019 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "TimedConnect.hpp"

#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/system_error.hpp>

#include <string>


#include <boost/asio/yield.hpp>

// In this example, the composed operation's logic is implemented as a state
// machine within a hand-crafted function object.
struct async_socks4_handshake_implementation {

	// The implementation holds a reference to the socket as it is used for
	// multiple async_write operations.
	boost::asio::ip::tcp::socket    &m_socket;
	
	std::unique_ptr<unsigned char[]> m_request_buffer;   // length 14, for all I know
	const unsigned int               m_request_length;

	std::unique_ptr<unsigned char[]> m_reply_buffer;     // always length 8
	const unsigned int               m_reply_length;

	// A steady timer used for introducing a delay.
	// Will use that for timeout
//	std::unique_ptr<boost::asio::steady_timer> m_delay_timer;

	// The coroutine state.
	boost::asio::coroutine           m_coro;

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
			std::size_t = 0) {

		using namespace boost::asio::ip;

		enum class ReplyStatus {
			request_granted = 0x5a,
			request_failed = 0x5b,
			request_failed_no_identd = 0x5c,
			request_failed_bad_user_id = 0x5d
		};

		// This is the coroutine impl. Basically a long switch clause which jumps 
		// back in where yield was last called .
		// Every time an async ops inside yields, the state is saved for the next call.
		// Error is the global n_error above
		reenter(m_coro) {

			// Put the timeout in ASAP!
// 			m_delay_timer->expires_after(std::chrono::seconds(1));
// 			yield m_delay_timer->async_wait(std::move(n_self));
// 			if (n_error) {
// 				break;
// 			}

			yield boost::asio::async_write(m_socket,
						boost::asio::const_buffer(m_request_buffer.get(), m_request_length),
						std::move(n_self));
			if (n_error) {
				n_self.complete(n_error);
				return;
			}
		
			yield boost::asio::async_read(m_socket,
						boost::asio::mutable_buffer(m_reply_buffer.get(), m_reply_length),
						boost::asio::transfer_at_least(m_reply_length),
						std::move(n_self));
			if (n_error) {
				n_self.complete(n_error);
				return;
			}

			if (m_reply_buffer[0] != 0) {
				n_self.complete(boost::system::errc::make_error_code(boost::system::errc::protocol_not_supported));
				return;
			}

			boost::system::error_code return_code;

			switch (static_cast<ReplyStatus>(m_reply_buffer[1])) {
				case ReplyStatus::request_granted:
					m_request_buffer.reset();
					m_reply_buffer.reset();
					n_self.complete(n_error);
					return;

				default:
				case ReplyStatus::request_failed:
					return_code = boost::system::errc::make_error_code(boost::system::errc::interrupted);
					break;

				case ReplyStatus::request_failed_no_identd:
					return_code = boost::system::errc::make_error_code(boost::system::errc::operation_not_permitted);
					break;

				case ReplyStatus::request_failed_bad_user_id:
					return_code = boost::system::errc::make_error_code(boost::system::errc::operation_not_permitted);
					break;
			}

			// In all cases except request_granted the server closes the connection.
			// I do the same with the socket
			boost::system::error_code ignored_errc;
			m_socket.close(ignored_errc);

			// Deallocate the buffers before calling the
			// user-supplied completion handler.
			// The example this was taken from here:
			// https://www.boost.org/doc/libs/1_71_0/doc/html/boost_asio/example/cpp11/operations/composed_8.cpp
			// ...does this explicitly. I don't know why, shouldn't they go out of scope anyway?
	
			m_request_buffer.reset();
			m_reply_buffer.reset();

			// Call the user-supplied handler with the result of the operation.
			n_self.complete(return_code);
		}
	}
};

#include <boost/asio/unyield.hpp>


// strangely, I cannot have above struct in the namespace. Apparently due to the yield/unyield magick.

namespace moose {
namespace tools {

/*! @brief a socks4 handshake wrapped up in an asio composed operation
	
	@param n_socket assumed to be already connected to the proxy
	@param n_target_ip the ip you want the proxy to connect to (SOCKS4 only supports IPv4)
	@param n_target_port the target port for your connection
	@param n_token the completion token will be called when the operation is finished.
				Should be of Type: void (boost::system::error_code) noexcept {}
		
 */
template <typename CompletionToken>
auto async_socks4_handshake(boost::asio::ip::tcp::socket &n_socket,
		const boost::asio::ip::address_v4 n_target_ip, const boost::uint16_t n_target_port,
		CompletionToken &&n_token)

	// See https://www.boost.org/doc/libs/1_71_0/doc/html/boost_asio/example/cpp11/operations/composed_8.cpp
	// for details of what this is.
	// Use of auto as well as trailing return type seem  necessary as the type is dependent on CompletionToken. 
	// It wouldn't compile otherwise
	-> typename boost::asio::async_result<
			typename std::decay<CompletionToken>::type,
			void (boost::system::error_code)
	>::return_type {

	using namespace boost::asio::ip;

	// Moose, listen up!
	//
	// I think the reason this impl here exists is primarily to setup local data and 
	// objects in the impl wrapper. As it turns out, inside the coroutine impl no objects can be created on stack.
	// Possibly because the coroutine is 'stackless'. But anyway, objects should be set up here and moved into the impl.
	// So, every object you need inside the impl must be created here.

	boost::asio::ip::address_v4::bytes_type byteaddress = n_target_ip.to_bytes();

	// The length of our request is always the same, 14 bytes. May vary only with user ID string
	constexpr unsigned int request_length = 14;
	constexpr unsigned int reply_length = 8;

	std::unique_ptr<unsigned char []> request_buffer(new unsigned char [request_length]);

	request_buffer[0] = static_cast<std::uint8_t>(4);              // SOCKS protocol version 4
	request_buffer[1] = static_cast<std::uint8_t>(1);              // connect command
	request_buffer[2] = (n_target_port >> 8) & 0xff;               // port high byte
	request_buffer[3] = n_target_port & 0xff;                      // port low byte
	memcpy(&request_buffer[4], &byteaddress, byteaddress.size());  // 4 bytes for the IP, only v4 supported by... well... v4
	strcpy(reinterpret_cast<char *>(&request_buffer[8]), "Stage"); // user identification may be anything

	std::unique_ptr<unsigned char[]> reply_buffer(new unsigned char[reply_length]);


	// Create a steady_timer to be used for the delay between messages.
// 	std::unique_ptr<boost::asio::steady_timer> m_delay_timer(
// 			new boost::asio::steady_timer(n_socket.get_executor()));

	return boost::asio::async_compose<CompletionToken, void (boost::system::error_code)>(
			async_socks4_handshake_implementation{			
					std::ref(n_socket),
					std::move(request_buffer),
					request_length,
					std::move(reply_buffer),
					reply_length,
					boost::asio::coroutine() },
			n_token, n_socket);
}

#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void Socks4getRidOfLNK4221();
#endif

}
}

