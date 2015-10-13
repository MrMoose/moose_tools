//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define MOOSE_TOOLS_DLL

// see https://gcc.gnu.org/wiki/Visibility

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
	#define MOOSE_TOOLS_DLL_IMPORT __declspec(dllimport)
	#define MOOSE_TOOLS_DLL_EXPORT __declspec(dllexport)
	#define MOOSE_TOOLS_DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define MOOSE_TOOLS_DLL_IMPORT __attribute__ ((visibility ("default")))
		#define MOOSE_TOOLS_DLL_EXPORT __attribute__ ((visibility ("default")))
		#define MOOSE_TOOLS_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
		#define MOOSE_TOOLS_DLL_IMPORT
		#define MOOSE_TOOLS_DLL_EXPORT
		#define MOOSE_TOOLS_DLL_LOCAL
	#endif
#endif

#ifdef MOOSE_TOOLS_DLL // defined if is compiled as a DLL
	#ifdef moose_tools_EXPORTS // defined if we are building the DLL (instead of using it)
		#define MOOSE_TOOLS_API MOOSE_TOOLS_DLL_EXPORT
	#else
		#define MOOSE_TOOLS_API MOOSE_TOOLS_DLL_IMPORT
	#endif // moose_tools_EXPORTS
	#define MOOSE_TOOLS_LOCAL MOOSE_TOOLS_DLL_LOCAL
#else // MOOSE_TOOLS_DLL is not defined: this means it is a static lib.
	#define MOOSE_TOOLS_API
	#define MOOSE_TOOLS_LOCAL
#endif // MOOSE_TOOLS_DLL




