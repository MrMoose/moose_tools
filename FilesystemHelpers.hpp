
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

namespace moose {
namespace tools {

/*! Found at https://stackoverflow.com/questions/15541263/how-to-determine-if-file-is-contained-by-path-with-boost-filesystem-v3
	(c) by Rob Kennedy.

	Distributed at Stackoverflow: User contributions licensed under CC By-SA 3.0
*/
MOOSE_TOOLS_API bool path_contains_file(const boost::filesystem::path &n_dir, const boost::filesystem::path &n_file);

}
}

