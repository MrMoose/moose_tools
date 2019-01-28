
# Copyright 2015 Stephan Menzel. Distributed under the Boost
# Software License, Version 1.0. (See accompanying file
# LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.6)
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
	FilesystemHelpers.hpp
	)

set(MOOSE_TOOLS_EVENT_LOG FALSE CACHE BOOL "Set to true if you want to log to event log in Windows")
set(MOOSE_TOOLS_CONSOLE_LOG TRUE CACHE BOOL "Set to true if you want to log to log to console stdout")
set(MOOSE_TOOLS_FILE_LOG TRUE CACHE BOOL "Set to true if for file log out to default.log")

add_library(moose_tools ${MOOSE_TOOLS_SRC} ${MOOSE_TOOLS_HDR})
if (${BUILD_SHARED_LIBS})
	target_compile_definitions(moose_tools PUBLIC -DMOOSE_TOOLS_DLL)
endif()

target_link_libraries(moose_tools 
	PUBLIC
		Boost::boost
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

set_property(TARGET moose_tools PROPERTY CXX_STANDARD 14)
if (${MOOSE_USE_COTIRE})
	cotire(moose_tools)
endif()

add_subdirectory(test)
enable_testing()
add_test(NAME Random      COMMAND TestRandom     )
add_test(NAME ThreadedId  COMMAND TestThreadId   )
add_test(NAME String      COMMAND TestString     )
add_test(NAME IdTagged    COMMAND TestIdTagged   )
add_test(NAME Mutexed     COMMAND TestMutexed    )
add_test(NAME AsioHelpers COMMAND TestAsioHelpers)

