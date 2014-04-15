// Copyright 2014 Quartz Technologies, Ltd. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Quartz Technologies Ltd. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * This file is based on boost python binding by David Abrahams (2002)
 * that was distributed under the Boost Software License, Version 1.0.
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef v8bridge_config_hpp
#   define v8bridge_config_hpp

#   include <boost/config.hpp>
#   include <boost/detail/workaround.hpp>

#   if defined(BOOST_MSVC)

#       if _MSC_VER < 1300
#           define BOOST_MSVC6_OR_EARLIER 1
#       endif

#       pragma warning (disable : 4786) // disable truncated debug symbols
#       pragma warning (disable : 4251) // disable exported dll function
#       pragma warning (disable : 4800) //'int' : forcing value to bool 'true' or 'false'
#       pragma warning (disable : 4275) // non dll-interface class

#   elif defined(__ICL) && __ICL < 600 // Intel C++ 5

#       pragma warning(disable: 985) // identifier was truncated in debug information

#   endif

// The STLport puts all of the standard 'C' library names in std (as far as the
// user is concerned), but without it you need a fix if you're using MSVC or
// Intel C++

#   if defined(BOOST_NO_STDC_NAMESPACE)
#       define BOOST_CSTD_
#   else
#       define BOOST_CSTD_ std
#   endif

/*****************************************************************************
 *
 *  Set up dll import/export options:
 *
 ****************************************************************************/

// backwards compatibility:
#   ifdef V8_STATIC_LIB
#       define V8_STATIC_LINK
#   elif !defined(V8_DYNAMIC_LIB)
#       define V8_DYNAMIC_LIB
#   endif

#   if defined(V8_DYNAMIC_LIB)

#       if !defined(_WIN32) && !defined(__CYGWIN__) \
                && !defined(V8_USE_GCC_SYMBOL_VISIBILITY) \
                && BOOST_WORKAROUND(__GNUC__, >= 3) && (__GNUC_MINOR__ >=5 || __GNUC__ > 3)
#           define V8_USE_GCC_SYMBOL_VISIBILITY 1
#       endif

#       if V8_USE_GCC_SYMBOL_VISIBILITY
#           if defined(V8_SOURCE)
#              define V8_DECL __attribute__ ((__visibility__("default")))
#              define V8_BUILD_DLL
#           else
#              define V8_DECL
#           endif

#           define V8_DECL_FORWARD
#           define V8_DECL_EXCEPTION __attribute__ ((__visibility__("default")))
#       elif (defined(_WIN32) || defined(__CYGWIN__))
#           if defined(V8_SOURCE)
#               define V8_DECL __declspec(dllexport)
#               define V8_BUILD_DLL
#           else
#               define V8_DECL __declspec(dllimport)
#           endif
#       endif
#   endif

#   ifndef V8_DECL
#       define V8_DECL
#   endif

#   ifndef V8_DECL_FORWARD
#       define V8_DECL_FORWARD V8_DECL
#   endif

#   ifndef V8_DECL_EXCEPTION
#       define V8_DECL_EXCEPTION V8_DECL
#   endif

#   if BOOST_WORKAROUND(__DECCXX_VER, BOOST_TESTED_AT(60590042))
    // Replace broken Tru64/cxx offsetof macro
#       define V8_OFFSETOF(s_name, s_member) ((size_t)__INTADDR__(&(((s_name *)0)->s_member)))
#   else
#       define V8_OFFSETOF offsetof
#   endif

//  enable automatic library variant selection  ------------------------------//
#   if !defined(V8_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(V8_NO_LIB)

//
// Set the name of our library, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#       define BOOST_LIB_NAME V8Bridge

//
// If we're importing code from a dll, then tell auto_link.hpp about it:
//
#       ifdef V8_DYNAMIC_LIB
#           define BOOST_DYN_LINK
#       endif

//
// And include the header that does the work:
// * You should remove this comment line in case you do link your project with boost config library.
//
//#       include <boost/config/auto_link.hpp>
#   endif  // auto-linking disabled

#endif