
//  Copyright 2018 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE AsioHelpersTests
#include <boost/test/unit_test.hpp>

#include "../AsioHelpers.hpp"
#include "../TimedConnect.hpp"

#include <boost/iostreams/stream.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include <thread>
#include <memory>

namespace asio      = boost::asio;
namespace iostreams = boost::iostreams;

using moose::tools::asio_streambuf_input_device;

BOOST_AUTO_TEST_CASE(CheckStreambufLimit) {

	char test[128];
	memset(test, 0, 128);
	asio::streambuf sbuf;

	{
		// fill streambuf with some characters
		std::iostream sin(&sbuf);
		sin << "Hello World!";
	}

	{
		// construct limited stream to extract from
		iostreams::stream<asio_streambuf_input_device> os(sbuf, 5);

		// I expect this to read only 5 because limit
		os.read(test, 127);

		BOOST_CHECK(boost::algorithm::equals(test, "Hello"));
	}

	{
		// put more characters 
		std::iostream sin(&sbuf);
		sin << " This is a Test";
	}

	{
		// now for a second time
		iostreams::stream<asio_streambuf_input_device> os(sbuf, 5);
		BOOST_CHECK(os.good());
		BOOST_CHECK(!os.eof());
		BOOST_CHECK(!os.bad());
	//	BOOST_CHECK(!os.fail());

		// I expect this to read only 5 because limit
		os.read(test, 127);

		BOOST_CHECK(boost::algorithm::equals(test, " Worl"));
		BOOST_CHECK(os.eof());
		BOOST_CHECK(!os.bad());
	//	BOOST_CHECK(!os.fail());
	}
}




/* imported async echo server from asio examples just to have something to connect to

 */

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
	public:
		session(tcp::socket socket) : socket_(std::move(socket)) {
		}

		void start() {
			do_read();
		}

	private:
		void do_read() {
			auto self(shared_from_this());
			socket_.async_read_some(boost::asio::buffer(data_, max_length),
				[this, self](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					do_write(length);
				}
			});
		}

		void do_write(std::size_t length) {
			auto self(shared_from_this());
			boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					do_read();
				}
			});
		}

		tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];
};

class server {
	public:
		server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
			do_accept();
		}

	private:
		void do_accept() {

			acceptor_.async_accept(
				[this](boost::system::error_code ec, tcp::socket socket) {
				if (!ec) {
					std::make_shared<session>(std::move(socket))->start();
				}

				do_accept();
			});
		}

		tcp::acceptor acceptor_;
};


struct ServerFixture {

	ServerFixture() {

		BOOST_TEST_MESSAGE("setup tcp connect fixture. Just to have something to connect to");

		m_server_thread = std::make_unique<boost::thread>([this] {		
		
			m_server = std::make_unique<server>(m_io_ctx, 2000);
			m_io_ctx.run();
		});
	}

	~ServerFixture() {

		BOOST_TEST_MESSAGE("teardown tcp connect fixture");
	
		m_io_ctx.stop();
		m_server_thread->join();
		m_server_thread.reset();
	}

	boost::asio::io_context        m_io_ctx;
	
	std::unique_ptr<boost::thread> m_server_thread;
	std::unique_ptr<server>        m_server;
};

BOOST_FIXTURE_TEST_SUITE(connections, ServerFixture)

BOOST_AUTO_TEST_CASE(TimedConnect) {

	// First see if a normal successful connect works.
	// Obviously this means we have to connect somewhere.
	// So I'm starting a thread and run that simple echo server in it
	BOOST_TEST_MESSAGE("testing timed connect operation");

	boost::asio::io_context io_ctx;
	boost::asio::ip::tcp::socket socket(io_ctx);
	boost::asio::executor_work_guard<
		boost::asio::io_context::executor_type> work{ boost::asio::make_work_guard(io_ctx) };
	std::unique_ptr<boost::thread> iothread{ std::make_unique<boost::thread>([&] { io_ctx.run(); } ) };

	boost::condition_variable cond;
	boost::mutex mutex;
	bool connect_ready = false;

	// Handler will set this to the error that came in
	boost::system::error_code error;

	moose::tools::async_timed_connect<4>(socket, "127.0.0.1", 2000, 5,

		[&] (boost::system::error_code n_errc) {

			// I just relay whatever error came up and be on my way
			error = n_errc;
			boost::lock_guard<boost::mutex> lock(mutex);
			connect_ready = true;
			cond.notify_one();
	});

	// Wait for the condition variable to turn true as the connect operation
	// succeeds or fails in the background
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		while (!connect_ready) {
			cond.wait(lock);
		}
	}
	
	BOOST_TEST_MESSAGE("Checking connect results");

	// We should have no error and a connected socket

	BOOST_CHECK(error == boost::system::error_code());
	BOOST_CHECK(socket.is_open());
	BOOST_CHECK(socket.remote_endpoint().address() == boost::asio::ip::address::from_string("127.0.0.1"));

	socket.close();
	work.reset();
	io_ctx.stop();
	iothread->join();
	iothread.reset();
}


using fsec = std::chrono::duration<float>;

BOOST_AUTO_TEST_CASE(TimeoutConnect) {

	// Now specifically check the timeout case
	BOOST_TEST_MESSAGE("testing timed connect operation");

	boost::asio::io_context io_ctx;
	boost::asio::ip::tcp::socket socket(io_ctx);
	boost::asio::executor_work_guard<
		boost::asio::io_context::executor_type> work{ boost::asio::make_work_guard(io_ctx) };
	std::unique_ptr<boost::thread> iothread{ std::make_unique<boost::thread>([&] { io_ctx.run(); }) };

	boost::condition_variable cond;
	boost::mutex mutex;
	bool connect_ready = false;

	// Handler will set this to the error that came in
	boost::system::error_code error;

	// Measure the time it takes for the operation to complete
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	moose::tools::async_timed_connect(socket, "127.0.0.1", 2001, 2,

		[&](boost::system::error_code n_errc) {

			// I just relay whatever error came up and be on my way
			error = n_errc;
			boost::lock_guard<boost::mutex> lock(mutex);
			connect_ready = true;
			cond.notify_one();
	});

	// Wait for the condition variable to turn true as the connect operation
	// succeeds or fails in the background
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		while (!connect_ready) {
			cond.wait(lock);
		}
	}

	// If we are way past our deadline right now the timeout must be considered faulty
	const fsec dur = (std::chrono::steady_clock::now() - start);
	if (dur > std::chrono::milliseconds(1900) && dur < std::chrono::milliseconds(2100)) {
		std::cout << "Connection timeout worked OK after " << dur.count() << " secs: " << error << std::endl;
	} else {
		std::cerr << "Connection timeout unexpected, exception after " << dur.count() << " secs: " << error << std::endl;
	}

	BOOST_TEST_MESSAGE("Checking connect results");

	// We should have a timeout error

	BOOST_CHECK(error == boost::asio::error::timed_out);
	BOOST_CHECK(!socket.is_open());

	io_ctx.stop();
	iothread->join();
	iothread.reset();
}


BOOST_AUTO_TEST_CASE(WrongProtocol) {

	// Now specifically check the timeout case
	BOOST_TEST_MESSAGE("testing wrong protocol selection");

	boost::asio::io_context io_ctx;
	boost::asio::ip::tcp::socket socket(io_ctx);
	boost::asio::executor_work_guard<
			boost::asio::io_context::executor_type> work{ boost::asio::make_work_guard(io_ctx) };
	std::unique_ptr<boost::thread> iothread{ std::make_unique<boost::thread>([&] { io_ctx.run(); }) };

	boost::condition_variable cond;
	boost::mutex mutex;
	bool connect_ready = false;

	// Handler will set this to the error that came in
	boost::system::error_code error;

	moose::tools::async_timed_connect<6>(socket, "127.0.0.1", 2001, 2,

		[&](boost::system::error_code n_errc) {

			// I just relay whatever error came up and be on my way
			error = n_errc;
			boost::lock_guard<boost::mutex> lock(mutex);
			connect_ready = true;
			cond.notify_one();
	});

	// Wait for the condition variable to turn true as the connect operation
	// succeeds or fails in the background
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		while (!connect_ready) {
			cond.wait(lock);
		}
	}

	BOOST_TEST_MESSAGE("Checking connect results");

	// We should have no resolved endpoints due to wrong protocol. "127.0.0.1" will not resolve to v6
	BOOST_CHECK(error);
	BOOST_CHECK(!socket.is_open());

	io_ctx.stop();
	iothread->join();
	iothread.reset();
}


BOOST_AUTO_TEST_SUITE_END()
