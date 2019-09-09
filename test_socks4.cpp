
//  Copyright 2019 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "tools/SOCKS4.hpp"
#include "tools/Log.hpp"
#include "tools/Error.hpp"
#include "tools/Random.hpp"

#include <boost/config.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/program_options.hpp>

#ifndef _WIN32
#include <sys/resource.h>
#endif

#include <iostream>
#include <cstdlib>
#include <string>

using namespace moose::tools;
using boost::asio::ip::tcp;   // Despite popular opinion, tcp is not a namespace ;-)

int main(int argc, char **argv) {
	
#ifndef _WIN32
	// core dumps may be disallowed by parent of this process; change that
	struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);
#endif

	moose::tools::init_logging();

	namespace po = boost::program_options;

	po::options_description desc("socks4 test options");
	desc.add_options()
		("help,h", "Print this help message")
		("socks_host,s", po::value<std::string>()->default_value("master.vr-on.cloud"), "give running socks4 server hostname")
		("socks_port,p", po::value<boost::uint16_t>()->default_value(1080), "give running socks4 server port to use")
		("target_host,S", po::value<std::string>()->default_value("www.google.de"), "give target server hostname")
		("target_port,P", po::value<boost::uint16_t>()->default_value(80), "give target server port")
		;

	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << "Behold your options!\n";
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		const std::string socks_hostname = vm["socks_host"].as<std::string>();
		const boost::uint16_t socks_port = vm["socks_port"].as<boost::uint16_t>();
		const std::string target_hostname = vm["target_host"].as<std::string>();
		const boost::uint16_t target_port = vm["target_port"].as<boost::uint16_t>();

		std::cout << "Setup test environment..." << std::endl;

		boost::asio::io_context io_ctx;
		tcp::socket socket(io_ctx);
		boost::asio::executor_work_guard<
			boost::asio::io_context::executor_type> work{ boost::asio::make_work_guard(io_ctx) };
		std::unique_ptr<boost::thread> iothread{ std::make_unique<boost::thread>([&] { io_ctx.run(); }) };

		boost::condition_variable cond;
		boost::mutex mutex;
		bool connect_ready = false;

		// Handler will set this to the error that came in
		boost::system::error_code error;

		std::cout << "Connect to SOCKS4 proxy at " << socks_hostname << ":" << socks_port << std::endl;

		moose::tools::async_timed_connect(socket, socks_hostname, socks_port, 5,

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

		// We should have no error and a connected socket

		if (socket.is_open() && !error) {
			boost::system::error_code ignored;
			tcp::endpoint remote_ep{ socket.remote_endpoint(ignored) };
			std::cout << "Connection established to " << remote_ep << std::endl;
		} else {
			std::cerr << "Connection failed: " << error << std::endl;
			return EXIT_FAILURE;
		}
		
		connect_ready = false;

		std::cout << "Starting handshake for target:  " << socks_hostname << ":" << socks_port << std::endl;


		moose::tools::async_socks4_handshake(socket, target_hostname, target_port, 5,

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

		if (socket.is_open() && !error) {
			std::cout << "Handshake done, connection should remote now to " << target_hostname << ":" << target_port << std::endl;
		} else {
			std::cerr << "Handshake failed: " << error.message() << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Starting handshake for target:  " << socks_hostname << ":" << socks_port << std::endl;





		socket.close();
		work.reset();
		io_ctx.stop();
		iothread->join();
		iothread.reset();








		std::cout << "done, all tests passed" << std::endl;

		return EXIT_SUCCESS;

	} catch (const moose_error &merr) {
		std::cerr << "Exception executing test cases: " << boost::diagnostic_information(merr) << std::endl;
	} catch (const std::exception &sex) {
		std::cerr << "Unexpected exception reached main: " << sex.what() << std::endl;
	} catch (...) {
		std::cerr << "Unhandled error reached main function. Aborting" << std::endl;
	}

	return EXIT_FAILURE;
}
