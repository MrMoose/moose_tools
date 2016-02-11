
# Copyright 2015 Stephan Menzel. Distributed under the Boost
# Software License, Version 1.0. (See accompanying file
# LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(TestRandom TestRandom.cpp)
target_link_libraries(TestRandom moose_tools ${Boost_LIBRARIES})

add_executable(TestThreadId TestThreadId.cpp)
target_link_libraries(TestThreadId moose_tools ${Boost_LIBRARIES})
