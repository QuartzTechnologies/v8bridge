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

#ifndef BOOST_PP_IS_ITERATING
#   ifndef v8bridge_type_list_h
#       define v8bridge_type_list_h

#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/detail/preprocessor.hpp>

#       if V8_MAX_ARITY + 2 > V8_MAX_BASES
#           define V8_BRIDGE_LIST_SIZE BOOST_PP_INC(BOOST_PP_INC(V8_MAX_ARITY))
#       else
#           define V8_BRIDGE_LIST_SIZE V8_MAX_BASES
#       endif

#       include <boost/preprocessor/enum_params.hpp>
#       include <boost/preprocessor/enum_params_with_a_default.hpp>
#       include <boost/preprocessor/repetition/enum.hpp>
#       include <boost/preprocessor/comma_if.hpp>
#       include <boost/preprocessor/arithmetic/sub.hpp>
#       include <boost/preprocessor/iterate.hpp>
#       include <boost/preprocessor/repetition/enum_trailing.hpp>

// Compute the MPL vector header to use for lists up to BOOST_PYTHON_LIST_SIZE in length
#       if V8_BRIDGE_LIST_SIZE > 48
#           error Arities above 48 not supported by V8 Bridge due to MPL internal limit
#       elif V8_BRIDGE_LIST_SIZE > 38
#           include <boost/mpl/vector/vector50.hpp>
#       elif V8_BRIDGE_LIST_SIZE > 28
#           include <boost/mpl/vector/vector40.hpp>
#       elif V8_BRIDGE_LIST_SIZE > 18
#           include <boost/mpl/vector/vector30.hpp>
#       elif V8_BRIDGE_LIST_SIZE > 8
#           include <boost/mpl/vector/vector20.hpp>
#       else
#           include <boost/mpl/vector/vector10.hpp>
#       endif

namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            template <BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(V8_BRIDGE_LIST_SIZE, class TType, mpl::void_)>
            struct type_list : BOOST_PP_CAT(mpl::vector, V8_BRIDGE_LIST_SIZE)<BOOST_PP_ENUM_PARAMS_Z(1, V8_BRIDGE_LIST_SIZE, TType) >
            {
            };
            
#       define BOOST_PP_ITERATION_PARAMS_1 (3, (0, BOOST_PP_DEC(V8_BRIDGE_LIST_SIZE), <v8bridge/detail/type_list.hpp>))
#       include BOOST_PP_ITERATE()
        }
    }
}

#   endif // v8bridge_type_list_h
#else // BOOST_PP_IS_ITERATING
#   define N BOOST_PP_ITERATION()
#   define V8_BRIDGE_ITER_VOID_ARGS BOOST_PP_SUB_D(1, V8_BRIDGE_LIST_SIZE, N)

template <BOOST_PP_ENUM_PARAMS_Z(1, N, class TType) >
struct type_list<
BOOST_PP_ENUM_PARAMS_Z(1, N, TType)
BOOST_PP_COMMA_IF(N)
BOOST_PP_ENUM(
              V8_BRIDGE_ITER_VOID_ARGS, V8_FIXED, mpl::void_)
>
: BOOST_PP_CAT(mpl::vector, N)<BOOST_PP_ENUM_PARAMS_Z(1, N, TType)>
{
};

# undef V8_BRIDGE_ITER_VOID_ARGS
# undef N

#endif
