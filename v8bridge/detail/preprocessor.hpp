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

#ifndef v8bridge_preprocessor_hpp
#   define v8bridge_preprocessor_hpp

#   include <boost/preprocessor/cat.hpp>
#   include <boost/preprocessor/comma_if.hpp>
#   include <boost/preprocessor/repeat.hpp>
#   include <boost/preprocessor/tuple/elem.hpp>

// stuff that should be in the preprocessor library

#   define V8_APPLY(x) BOOST_PP_CAT(V8_APPLY_, x)
#   define V8_APPLY_V8_ITEM(v) v
#   define V8_APPLY_V8_NIL

// cv-qualifiers

#   if !defined(__MWERKS__) || __MWERKS__ > 0x2407
#       define V8_CV_COUNT 4
#   else
#       define V8_CV_COUNT 1
#   endif

#   ifndef V8_MAX_ARITY
#       define V8_MAX_ARITY 15
#   endif

#   ifndef V8_MAX_BASES
#       define V8_MAX_BASES 10
#   endif

#   define V8_CV_QUALIFIER(i)     V8_APPLY(BOOST_PP_TUPLE_ELEM(4, i, V8_CV_QUALIFIER_I))

#   define V8_CV_QUALIFIER_I      \
                    (                           \
                        V8_NIL,                 \
                        V8_ITEM(const),         \
                        V8_ITEM(volatile),      \
                        V8_ITEM(const volatile) \
                    )

// enumerators
#   define V8_UNARY_ENUM(c, text) BOOST_PP_REPEAT(c, V8_UNARY_ENUM_I, text)
#   define V8_UNARY_ENUM_I(z, n, text) BOOST_PP_COMMA_IF(n) text ## n

#   define V8_BINARY_ENUM(c, a, b) BOOST_PP_REPEAT(c, V8_BINARY_ENUM_I, (a, b))
#   define V8_BINARY_ENUM_I(z, n, _) BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 0, _), n) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, _), n)

#   define V8_ENUM_WITH_DEFAULT(c, text, def) BOOST_PP_REPEAT(c, V8_ENUM_WITH_DEFAULT_I, (text, def))
#   define V8_ENUM_WITH_DEFAULT_I(z, n, _) BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 0, _), n) = BOOST_PP_TUPLE_ELEM(2, 1, _)

// fixed text (no commas)
#   define V8_FIXED(z, n, text) text

// flags
#   define V8_FUNCTION_POINTER    0x0001
#   define V8_POINTER_TO_MEMBER   0x0002

#endif
