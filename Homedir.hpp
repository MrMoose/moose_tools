
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

namespace moose {
namespace tools {

//! get the user home dir as shown by 
//! Fred Nurk in
//! http://stackoverflow.com/questions/4891006/how-to-create-a-folder-in-the-home-directory
//!
//! @return tries to do the job but falls back to "C:\" or "~" respectively on fail
//! @throw std::bad_alloc
boost::filesystem::path MOOSE_TOOLS_API user_home();

}
}

