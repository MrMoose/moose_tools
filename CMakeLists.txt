
# Copyright 2015 Stephan Menzel. Distributed under the Boost
# Software License, Version 1.0. (See accompanying file
# LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0)
project(moose_tools)

find_package(Boost 1.56.0 COMPONENTS random chrono thread system log REQUIRED)

set(MOOSE_TOOLS_SRC
	Random.cpp
	ThreadId.cpp
	Pimpled.cpp
	Mutexed.cpp
	)

set(MOOSE_TOOLS_HDR
	MooseToolsConfig.hpp
	Random.hpp
	ThreadId.hpp
	Pimpled.hpp
	Mutexed.hpp
	)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} SHARED ${MOOSE_TOOLS_SRC})

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=c++11 -fvisibility=hidden -fPIC -DPIC")
endif()

target_link_libraries(${PROJECT_NAME}
	${Boost_LIBRARIES}
)

add_subdirectory(test)
enable_testing()
add_test(NAME Random     COMMAND test/TestRandom  )
add_test(NAME ThreadedId COMMAND test/TestThreadId)


