
# Copyright 2015 Stephan Menzel. Distributed under the Boost
# Software License, Version 1.0. (See accompanying file
# LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_package (Boost COMPONENTS unit_test_framework REQUIRED)
#include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable(TestRandom TestRandom.cpp)
target_link_libraries(TestRandom moose_tools ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(TestThreadId TestThreadId.cpp)
target_link_libraries(TestThreadId moose_tools ${Boost_LIBRARIES})
