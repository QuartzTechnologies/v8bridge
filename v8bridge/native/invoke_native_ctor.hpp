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
 *  This file declares an interface to instanciate native classes using the supplied function arguments.
 *  See: invoke_native.hpp and invoke_v8_handle.hpp for similier approch references.
 *
 *  The following macros generate expansions for::
 *
 *      struct invoke_native_ctor<N>
 *      {
 *          template <class TClass, class TArgsList>
 *          struct apply
 *          {
 *              typedef typename mpl::begin<TArgsList ... iterN - 1>::type tN;
 *              typedef typename mpl::next<iterN>::type iterN;
 *
 *              static TClass *execute(t0 a0 ... tN aN)
 *              {
 *                  TClass *instance = NULL;
 *                  try
 *                  {
 *                      instance = (new TClass( a0 ... aN ));
 *                  }
 *                  catch (...)
 *                  {
 *                      delete instance;
 *                      throw;
 *                  }
 *
 *                  return instance;
 *              }
 *          }
 *      };
 */

#if !defined(BOOST_PP_IS_ITERATING)
#   ifndef v8bridge_invoke_native_ctor_hppp
#       define v8bridge_invoke_native_ctor_hpp

#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/detail/preprocessor.hpp>

#       include <v8bridge/detail/forward.hpp>

#       include <boost/mpl/next.hpp>
#       include <boost/mpl/begin_end.hpp>
#       include <boost/mpl/deref.hpp>

#       include <boost/preprocessor/iterate.hpp>
#       include <boost/preprocessor/iteration/local.hpp>
#       include <boost/preprocessor/repeat.hpp>
#       include <boost/preprocessor/debug/line.hpp>
#       include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>

#       include <cstddef>

namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            template <int nargs> struct invoke_native_ctor;
            
#  define V8_BRIDGE_DO_FORWARD_ARG(z, index, _) f##index(a##index)
            
            // specializations...
#  define BOOST_PP_ITERATION_PARAMS_1 (3, (0, V8_MAX_ARITY, <v8bridge/native/invoke_native_ctor.hpp>))
#  include BOOST_PP_ITERATE()
            
#  undef V8_BRIDGE_DO_FORWARD_ARG
        }
    }
}

#   endif // v8bridge_invoke_native_ctor_hppp
#else // BOOST_PP_IS_ITERATING
#   if BOOST_PP_ITERATION_DEPTH() == 1
#       if !(BOOST_WORKAROUND(__MWERKS__, > 0x3100) && BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3201)))
#           line BOOST_PP_LINE(__LINE__, invoke_native_ctor.hpp)
#       endif
#       define N BOOST_PP_ITERATION()

template <>
struct invoke_native_ctor<N>
{
    template <class TClass, class TArgsList>
    struct apply
    {
#       if N
        // Unrolled iteration through each argument type in ArgList,
        // choosing the type that will be forwarded on to the holder's
        // templated constructor.
        typedef typename mpl::begin<TArgsList>::type iter0;
        
#           define BOOST_PP_LOCAL_MACRO(n)               \
                    typedef typename mpl::deref<iter##n>::type t##n;        \
                    typedef typename mpl::next<iter##n>::type   \
                    BOOST_PP_CAT(iter,BOOST_PP_INC(n)); // Next iterator type
        
#           define BOOST_PP_LOCAL_LIMITS (0, N-1)
#           include BOOST_PP_LOCAL_ITERATE()
#       endif
        
        
        
        inline static TClass *execute(BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, t, a))
        {
            TClass *instance = NULL;
             try
             {
                 instance = (new TClass( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, a, BOOST_PP_INTERCEPT) ));
             }
            catch (...)
             {
                 delete instance;
                 throw;
             }
             
            return instance;
        }
    };
};

#       undef N
#   endif // BOOST_PP_ITERATION_DEPTH
#endif
