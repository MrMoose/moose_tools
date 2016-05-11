
# Copyright 2015 Stephan Menzel. Distributed under the Boost
# Software License, Version 1.0. (See accompanying file
# LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#cmake_minimum_required(VERSION 3.0)
#project(moose_tools)

#find_package(Boost 1.60.0 COMPONENTS random filesystem regex chrono thread system log REQUIRED)

set(MOOSE_TOOLS_SRC
	Random.cpp
	ThreadId.cpp
	Pimpled.cpp
	Mutexed.cpp
	Macros.cpp
	Error.cpp
	IdTagged.cpp
	String.cpp
	Homedir.cpp
	Log.cpp
	)

set(MOOSE_TOOLS_HDR
	MooseToolsConfig.hpp
	Random.hpp
	ThreadId.hpp
	Pimpled.hpp
	Mutexed.hpp
	Macros.hpp
	Error.hpp
	IdTagged.hpp
	String.hpp
	Homedir.hpp
	Log.hpp
	)

include_directories(
	SYSTEM
	${Boost_INCLUDE_DIRS}
)

include_directories(
	${moose_tools_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(MOOSE_TOOLS_EVENT_LOG FALSE CACHE BOOL "Set to true if you want to log to event log in Windows")

if (${MOOSE_TOOLS_EVENT_LOG})
	add_definitions(-DMOOSE_TOOLS_EVENT_LOG)
endif()

if (${UNREAL})
	add_library(moose_tools STATIC ${MOOSE_TOOLS_SRC} ${MOOSE_TOOLS_HDR})
else()
	add_definitions(-DMOOSE_TOOLS_DLL)
	add_library(moose_tools SHARED ${MOOSE_TOOLS_SRC} ${MOOSE_TOOLS_HDR})
endif()

target_link_libraries(moose_tools
	${Boost_LIBRARIES}
)



add_subdirectory(test)
enable_testing()
add_test(NAME Random     COMMAND TestRandom  )
add_test(NAME ThreadedId COMMAND TestThreadId)
add_test(NAME String     COMMAND TestString  )



