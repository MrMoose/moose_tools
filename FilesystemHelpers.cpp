
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "FilesystemHelpers.hpp"
#include "Assert.hpp"

#include <algorithm>

namespace moose {
namespace tools {

namespace fs = boost::filesystem;

bool path_contains_file(const boost::filesystem::path &n_dir, const boost::filesystem::path &n_file) {

	boost::filesystem::path dir(n_dir);
	boost::filesystem::path file(n_file);

	// If dir ends with "/" and isn't the root directory, then the final
	// component returned by iterators will include "." and will interfere
	// with the std::equal check below, so we strip it before proceeding.
	if (dir.filename() == ".") {
		dir.remove_filename();
	}

	// We're also not interested in the file's name.
	MOOSE_ASSERT(file.has_filename());
	file.remove_filename();

	// If dir has more components than file, then file can't possibly
	// reside in dir.
	const std::size_t dir_len = std::distance(dir.begin(), dir.end());
	const std::size_t file_len = std::distance(file.begin(), file.end());
	if (dir_len > file_len){
		return false;
	}

	// This stops checking when it reaches dir.end(), so it's OK if file
	// has more directory components afterward. They won't be checked.
	return std::equal(dir.begin(), dir.end(), file.begin());
}

}
}