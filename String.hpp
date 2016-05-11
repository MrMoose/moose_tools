
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <string>

namespace moose {
namespace tools {

//! truncate a string to at most n characters
void MOOSE_TOOLS_API truncate(std::string &n_string, std::size_t n_length) noexcept;

/*
  This should really be a template to fit into boost algos

template<typename SequenceT>
void trim(SequenceT &n_string);

}
*/

}
}

