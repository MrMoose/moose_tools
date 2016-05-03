
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "Homedir.hpp"

#include <stdlib.h>


namespace moose {
namespace tools {

namespace fs = boost::filesystem;

boost::filesystem::path user_home() {

	fs::path ret;

	// env USERPROFILE or if this fails, concatenate HOMEDRIVE+HOMEPATH
#if _WIN32

	const char *userprofile = getenv("USERPROFILE");
	if (userprofile) {
		ret = userprofile;
	} else {
		const char *homedrive = getenv("HOMEDRIVE");
		const char *homepath = getenv("HOMEPATH");
		if (homedrive && homepath) {
			ret = fs::path(homedrive) / fs::path(homepath);
		} else {
			ret = "C:\\";
		}
	}
#else
	const char *userhome = getenv("HOME");
	if (userhome) {
		ret = userhome;
	} else {
		ret = "~";
	}
#endif

	return ret;
}

}
}