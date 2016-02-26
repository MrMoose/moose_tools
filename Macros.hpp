
//  Copyright 2015 Stephan Menzel. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "MooseToolsConfig.hpp"

#include <boost/shared_ptr.hpp>

#include <memory>

#define MOOSE_FWD_DECLARE_CLASS(mt_macro_classname)                                \
class mt_macro_classname;                                                          \
typedef boost::shared_ptr<mt_macro_classname>       mt_macro_classname ## SPtr;    \
typedef boost::shared_ptr<const mt_macro_classname> mt_macro_classname ## CSPtr;   \
typedef boost::weak_ptr<mt_macro_classname>         mt_macro_classname ## WPtr;    \
typedef boost::weak_ptr<const mt_macro_classname>   mt_macro_classname ## CWPtr;   \
typedef std::unique_ptr<mt_macro_classname>         mt_macro_classname ## UPtr;



#define MOOSE_FWD_DECLARE_STRUCT(mt_macro_structname)                              \
struct mt_macro_structname;                                                        \
typedef boost::shared_ptr<mt_macro_structname>       mt_macro_structname ## SPtr;  \
typedef boost::shared_ptr<const mt_macro_structname> mt_macro_structname ## CSPtr; \
typedef boost::weak_ptr<mt_macro_structname>         mt_macro_structname ## WPtr;  \
typedef boost::weak_ptr<const mt_macro_structname>   mt_macro_structname ## CWPtr; \
typedef std::unique_ptr<mt_macro_structname>         mt_macro_structname ## UPtr;


