
# Copyright 2015 Stephan Menzel. Distributed under the Boost
# Software License, Version 1.0. (See accompanying file
# LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.12)
project(moose_tools)

find_package(Boost 1.68.0 REQUIRED COMPONENTS chrono thread system program_options log unit_test_framework)

set(MOOSE_TOOLS_SRC
	Carne.cpp
	Random.cpp
	ThreadId.cpp
	Pimpled.cpp
	Mutexed.cpp
	Macros.cpp
	Error.cpp
	IdTagged.cpp
	IdTaggedContainer.cpp
	String.cpp
	Homedir.cpp
	Log.cpp
	Assert.cpp
	AsioHelpers.cpp
	TimedConnect.cpp
	SOCKS4.cpp
	FilesystemHelpers.cpp
	)

set(MOOSE_TOOLS_HDR
	MooseToolsConfig.hpp
	Carne.hpp
	Random.hpp
	ThreadId.hpp
	Pimpled.hpp
	Mutexed.hpp
	Macros.hpp
	Error.hpp
	IdTagged.hpp
	IdTaggedContainer.hpp
	String.hpp
	Homedir.hpp
	Log.hpp
	Assert.hpp
	AsioHelpers.hpp
	TimedConnect.hpp
	SOCKS4.hpp
	FilesystemHelpers.hpp
	)

set(MOOSE_TOOLS_EVENT_LOG FALSE CACHE BOOL "Set to true if you want to log to event log in Windows")
set(MOOSE_TOOLS_CONSOLE_LOG TRUE CACHE BOOL "Set to true if you want to log to log to console stdout")
set(MOOSE_TOOLS_FILE_LOG TRUE CACHE BOOL "Set to true if for file log out to default.log")

add_library(moose_tools ${MOOSE_TOOLS_SRC} ${MOOSE_TOOLS_HDR})
if (${BUILD_SHARED_LIBS})
	target_compile_definitions(moose_tools PUBLIC -DMOOSE_TOOLS_DLL)
endif()


#if (NOT MSVC)
#	target_link_libraries(moose_tools ${CMAKE_DL_LIBS})
#endif()

target_link_libraries(moose_tools 
	PUBLIC
		Boost::boost
		${CMAKE_DL_LIBS}
	PRIVATE
		Boost::log
		Boost::regex
		Boost::thread
		Boost::filesystem
		Boost::system
)

target_include_directories(moose_tools
   PUBLIC 
        $<INSTALL_INTERFACE:tools>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (${CMAKE_VERSION} VERSION_LESS "3.16.0") 
	message("Please consider to switch to CMake 3.16.0 or above for PCH support")
else()
	target_precompile_headers(moose_tools
		PRIVATE
		"<string>"
		"<vector>"
		"<map>"
		"<iostream>"
		"<fstream>"
		"<algorithm>"
		"<boost/asio.hpp>"
		"<boost/chrono/chrono.hpp>"
		"<boost/thread/thread.hpp>"
		"<boost/spirit/include/qi.hpp>"
		"<boost/spirit/include/karma.hpp>"
		"<boost/spirit/include/phoenix.hpp>"
	)
	set_target_properties(moose_tools PROPERTIES UNITY_BUILD ON)
endif()

if (${MOOSE_TOOLS_EVENT_LOG})
	target_compile_definitions(moose_tools PRIVATE -DMOOSE_TOOLS_EVENT_LOG)
endif()

if (${MOOSE_TOOLS_CONSOLE_LOG})
	target_compile_definitions(moose_tools PRIVATE -DMOOSE_TOOLS_CONSOLE_LOG)
endif()

if (${MOOSE_TOOLS_FILE_LOG})
	target_compile_definitions(moose_tools PRIVATE -DMOOSE_TOOLS_FILE_LOG)
endif()

if (${BUILD_SHARED_LIBS})
	target_compile_definitions(moose_tools PUBLIC -DMOOSE_TOOLS_DLL)
endif()

set_property(TARGET moose_tools PROPERTY CXX_STANDARD 17)

add_executable(test_socks4 test_socks4.cpp)
target_link_libraries(test_socks4
	moose_tools
	Boost::thread
	Boost::filesystem
	Boost::system
	Boost::program_options
)

add_subdirectory(test)
enable_testing()
add_test(NAME Random      COMMAND TestRandom     )
add_test(NAME ThreadedId  COMMAND TestThreadId   )
add_test(NAME String      COMMAND TestString     )
add_test(NAME IdTagged    COMMAND TestIdTagged   )
add_test(NAME Mutexed     COMMAND TestMutexed    )
add_test(NAME AsioHelpers COMMAND TestAsioHelpers)

