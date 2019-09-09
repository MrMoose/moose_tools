
//  Copyright 2019 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"
#include "TimedConnect.hpp"
#include "Log.hpp"
#include "Assert.hpp"

#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/system/system_error.hpp>

#include <string>





// strangely, I cannot have above struct in the namespace. Apparently due to the yield/unyield magick.

namespace moose {
namespace tools {




// the composed operation's logic is implemented as a state
// machine. The coroutine approach didn't work as I could not adapt to different 
// handler signatures
struct async_socks4_handshake_implementation {

	enum class State {
		start,
		handle_resolve,
		handle_request_write,
		handle_read_response
	};

	enum class ReplyStatus {
		request_granted = 0x5a,
		request_failed = 0x5b,
		request_failed_no_identd = 0x5c,
		request_failed_bad_user_id = 0x5d
	};

	// The implementation holds a reference to the socket as it is used for
	// multiple async_write operations.
	boost::asio::ip::tcp::socket    &m_socket;
	boost::asio::ip::tcp::resolver   m_resolver;
	boost::asio::ip::tcp::resolver::query   m_query;
	State                            m_state;
	boost::uint16_t                  m_target_port;

	std::unique_ptr<unsigned char[]> m_request_buffer;   // length 14, for all I know
	unsigned int                     m_request_length;

	std::unique_ptr<unsigned char[]> m_reply_buffer;     // always length 8
	unsigned int                     m_reply_length;

	// A steady timer used for introducing a delay.
	// Will use that for timeout
//	std::unique_ptr<boost::asio::steady_timer> m_delay_timer;

	// The initializing function
	template <typename Self>
	void operator()(Self &n_self) {

		using namespace boost::asio::ip;
		using namespace moose::tools;

		MOOSE_ASSERT(m_state == State::start);

		m_state = State::handle_resolve;
		m_resolver.async_resolve(m_query, std::move(n_self));
	}

	// This handler signature shall be selected by async_resolve
	template <typename Self>
	void operator()(Self &n_self, const boost::system::error_code &n_error, boost::asio::ip::tcp::resolver::results_type n_results) {

		using namespace boost::asio::ip;
		using namespace moose::tools;

		if (n_error) {
			n_self.complete(n_error);
			return;
		}

		MOOSE_ASSERT(m_state == State::handle_resolve);

		boost::asio::ip::address_v4::bytes_type byteaddress;
		bool gotone = false;

		while (!gotone && (n_results != tcp::resolver::iterator())) {
			boost::asio::ip::address addr = (n_results++)->endpoint().address();
			if (addr.is_v6()) {
				BOOST_LOG_SEV(logger(), warning) << "IPv6 address resolved in SOCKS4 connect while we specifically asked for v4";
			} else {
				byteaddress = addr.to_v4().to_bytes();
				gotone = true;
			}
		}

		// If we have no v4 address returned there's little we can do.
		if (!gotone) {
			n_self.complete(boost::asio::error::host_not_found);
			return;
		}

		// Fill in the request buffer which was allocated in the function before.
		m_request_buffer[0] = static_cast<std::uint8_t>(4);              // SOCKS protocol version 4
		m_request_buffer[1] = static_cast<std::uint8_t>(1);              // connect command
		m_request_buffer[2] = (m_target_port >> 8) & 0xff;               // port high byte
		m_request_buffer[3] = m_target_port & 0xff;                      // port low byte
		memcpy(&m_request_buffer[4], &byteaddress, byteaddress.size());  // 4 bytes for the IP, only v4 supported by... well... v4
		strcpy(reinterpret_cast<char *>(&m_request_buffer[8]), "Stage"); // user identification may be anything	

		// write the request to the socket and start waiting for a response
		m_state = State::handle_request_write;
		boost::asio::async_write(m_socket,
				boost::asio::const_buffer(m_request_buffer.get(), m_request_length), std::move(n_self));
	}

	// This function is selected by both async_read and async_write, introducing the need for the state machine
	template <typename Self>
	void operator()(Self &n_self, boost::system::error_code n_error, std::size_t) {

		using namespace boost::asio::ip;
		using namespace moose::tools;

		if (m_state == State::handle_request_write) {

			if (n_error) {
				n_self.complete(n_error);
				return;
			}

			// We have successfully written our request and now receive a response
			m_state = State::handle_read_response;
			boost::asio::async_read(m_socket,
					boost::asio::mutable_buffer(m_reply_buffer.get(), m_reply_length),
					boost::asio::transfer_exactly(m_reply_length),
					std::move(n_self));
		} else {
			// We must be in that state or above logic contains a flaw
			MOOSE_ASSERT(m_state == State::handle_read_response);

			if (n_error) {
				n_self.complete(n_error);
				return;
			}

			// First byte is zero or error (wrong protocol)
			if (m_reply_buffer[0] != 0) {
				n_self.complete(boost::system::errc::make_error_code(boost::system::errc::protocol_not_supported));
				return;
			}

			// second byte is the actual response
			ReplyStatus status = static_cast<ReplyStatus>(m_reply_buffer[1]);

			if (status != ReplyStatus::request_granted) {
				// In all cases except request_granted the server closes the connection.
				// I do the same with the socket
				boost::system::error_code ignored_errc;
				m_socket.close(ignored_errc);

				if (status == ReplyStatus::request_failed) {
					n_error = boost::system::errc::make_error_code(boost::system::errc::interrupted);
				} else if (status == ReplyStatus::request_failed_no_identd) {
					n_error = boost::system::errc::make_error_code(boost::system::errc::operation_not_permitted);
				} else if (status == ReplyStatus::request_failed_bad_user_id) {
					n_error = boost::system::errc::make_error_code(boost::system::errc::operation_not_permitted);
				}
			}

			// Deallocate the buffers before calling the
			// user-supplied completion handler.
			// The example this was taken from here:
			// https://www.boost.org/doc/libs/1_71_0/doc/html/boost_asio/example/cpp11/operations/composed_8.cpp
			// ...does this explicitly. I don't know why, shouldn't they go out of scope anyway?
			m_request_buffer.reset();
			m_reply_buffer.reset();

			n_self.complete(n_error);
		}
	}

};







/*! @brief a socks4 handshake wrapped up in an asio composed operation
	
	@param n_socket assumed to be already connected to the proxy
	@param n_target_ip the ip you want the proxy to connect to (SOCKS4 only supports IPv4)
	@param n_target_port the target port for your connection
	@param n_token the completion token will be called when the operation is finished.
				Should be of Type: void (boost::system::error_code) noexcept {}
		
 */
template <typename CompletionToken>
auto async_socks4_handshake(boost::asio::ip::tcp::socket &n_socket,
		const std::string n_target_hostname, const boost::uint16_t n_target_port,
		CompletionToken &&n_token)

	// See https://www.boost.org/doc/libs/1_71_0/doc/html/boost_asio/example/cpp11/operations/composed_8.cpp
	// for details of what this is.
	// Use of auto as well as trailing return type seem  necessary as the type is dependent on CompletionToken. 
	// It wouldn't compile otherwise
	-> typename boost::asio::async_result<
			typename std::decay<CompletionToken>::type,
			void (boost::system::error_code)
	>::return_type {

	// Moose, listen up!
	//
	// I think the reason this impl here exists is primarily to setup local data and 
	// objects in the impl wrapper. As it turns out, inside the coroutine impl no objects can be created on stack.
	// Possibly because the coroutine is 'stackless'. But anyway, objects should be set up here and moved into the impl.
	// So, every object you need inside the impl must be created here.


	// resolver will get moved to the impl
	boost::asio::ip::tcp::resolver resolver{ n_socket.get_executor() };
	
	boost::asio::ip::tcp::resolver::query query{
		boost::asio::ip::tcp::v4(),
		n_target_hostname,
		"80",
		tcp::resolver::query::numeric_service
	};

	// The length of our request is always the same, 14 bytes. May vary only with user ID string
	constexpr unsigned int request_length = 14;
	constexpr unsigned int reply_length = 8;

	std::unique_ptr<unsigned char []> request_buffer(new unsigned char [request_length]);
	std::unique_ptr<unsigned char[]> reply_buffer(new unsigned char[reply_length]);

	// Create a steady_timer for timeouts
// 	std::unique_ptr<boost::asio::steady_timer> m_delay_timer(
// 			new boost::asio::steady_timer(n_socket.get_executor()));

	return boost::asio::async_compose<CompletionToken, void (boost::system::error_code)>(
			async_socks4_handshake_implementation{			
					std::ref(n_socket),
					std::move(resolver),
					std::move(query),
					async_socks4_handshake_implementation::State::start,
					n_target_port,
					std::move(request_buffer),
					request_length,
					std::move(reply_buffer),
					reply_length },
			n_token, n_socket);
}

#if defined(BOOST_MSVC)
MOOSE_TOOLS_API void Socks4getRidOfLNK4221();
#endif

}
}

