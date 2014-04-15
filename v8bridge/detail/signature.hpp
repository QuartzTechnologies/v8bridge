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

#if !defined(BOOST_PP_IS_ITERATING)
#   ifndef v8bridge_signature_hpp
#       define v8bridge_signature_hpp


#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/detail/preprocessor.hpp>

#       include <boost/mpl/if.hpp>
#       include <boost/type_traits/is_convertible.hpp>

#       include <boost/preprocessor/repeat.hpp>
#       include <boost/preprocessor/iterate.hpp>
#       include <boost/preprocessor/enum.hpp>
#       include <boost/preprocessor/enum_params.hpp>
#       include <boost/preprocessor/empty.hpp>
#       include <boost/preprocessor/arithmetic/sub.hpp>
#       include <boost/preprocessor/iterate.hpp>

#       include <boost/mpl/vector.hpp>

#       include <boost/preprocessor/arithmetic/sub.hpp>
#       include <boost/preprocessor/arithmetic/inc.hpp>
#       include <boost/preprocessor/repetition/enum_trailing_params.hpp>

#       define V8_LIST_INC(n)         BOOST_PP_CAT(mpl::vector, BOOST_PP_INC(n))

namespace v8 { namespace bridge {
    using namespace boost;
    
    // A metafunction returning C1 if C1 is derived from C2, and C2
    // otherwise
    template <class C1, class C2>
    struct most_derived
    {
        typedef typename mpl::if_<
        is_convertible<C1*,C2*>
        , C1
        , C2
        >::type type;
    };
    
    //  The following macros generate expansions for::
    //
    //      template <class TResult, class T0... class TN>
    //      inline mpl::vector<TResult, T0...TN>
    //      get_signature(TResult (V8_FN_CC *)(T0...TN), void* = 0)
    //      {
    //          return mpl::list<TResult, T0...TN>();
    //      }
    //
    //    where V8_FN_CC is a calling convention keyword, can be
    //
    //        empty, for default calling convention
    //        __cdecl (if V8_ENABLE_CDECL is defined)
    //        __stdcall (if V8_ENABLE_STDCALL is defined)
    //        __fastcall (if V8_ENABLE_FASTCALL is defined)
    //
    //   And, for an appropriate assortment of cv-qualifications::
    //
    //      template <class TResult, class TClass, class T0... class TN>
    //      inline mpl::vector<TResult, TClass&, T0...TN>
    //      get_signature(RT(V8_FN_CC TClass::*)(T0...TN) cv))
    //      {
    //          return mpl::list<TResult, TClass&, T0...TN>();
    //      }
    //
    //      template <class Target, class RT, class ClassT, class T0... class TN>
    //      inline mpl::vector<
    //          RT
    //        , typename most_derived<Target, ClassT>::type&
    //        , T0...TN
    //      >
    //      get_signature(RT(V8_FN_CC ClassT::*)(T0...TN) cv), Target*)
    //      {
    //          return mpl::list<RT, ClassT&, T0...TN>();
    //      }
    //
    //  There are two forms for invoking get_signature::
    //
    //      get_signature(f)
    //
    //  and ::
    //
    //      get_signature(f,(Target*)0)
    //
    //  These functions extract the return type, class (for member
    //  functions) and arguments of the input signature and stuff them in
    //  an mpl type sequence (the calling convention is dropped).
    //  Note that cv-qualification is dropped from
    //  the "hidden this" argument of member functions; that is a
    //  necessary sacrifice to ensure that an lvalue from_V8 converter
    //  is used.  A pointer is not used so that None will be rejected for
    //  overload resolution.
    //
    //  The second form of get_signature essentially downcasts the "hidden
    //  this" argument of member functions to Target, because the function
    //  may actually be a member of a base class which is not wrapped, and
    //  in that case conversion from V8 would fail.
    //
    
    
    // 'default' calling convention
    
#       define V8_FN_CC
#       define BOOST_PP_ITERATION_PARAMS_1 (3, (0, V8_MAX_ARITY, <v8bridge/detail/signature.hpp>))
#       include BOOST_PP_ITERATE()
#       undef V8_FN_CC
    // __cdecl calling convention
    
#       if defined(V8_ENABLE_CDECL)
#           define V8_FN_CC __cdecl
#           define V8_FN_CC_IS_CDECL
    
#           define BOOST_PP_ITERATION_PARAMS_1 (3, (0, V8_MAX_ARITY, <v8bridge/detail/signature.hpp>))
    
#           include BOOST_PP_ITERATE()
#           undef V8_FN_CC
#           undef V8_FN_CC_IS_CDECL
#       endif // defined(V8_ENABLE_CDECL)
    
    // __stdcall calling convention
#       if defined(V8_ENABLE_STDCALL)
#           define V8_FN_CC __stdcall
#           define BOOST_PP_ITERATION_PARAMS_1 (3, (0, V8_MAX_ARITY, <v8bridge/detail/signature.hpp>))
#           include BOOST_PP_ITERATE()
#           undef V8_FN_CC
#       endif // defined(V8_ENABLE_STDCALL)
    
    // __fastcall calling convention
    
#       if defined(V8_ENABLE_FASTCALL)
#           define V8_FN_CC __fastcall
#           define BOOST_PP_ITERATION_PARAMS_1 (3, (0, V8_MAX_ARITY, <v8bridge/detail/signature.hpp>))
#           include BOOST_PP_ITERATE()
#           undef V8_FN_CC
#       endif // defined(V8_ENABLE_FASTCALL)
#   undef V8_LIST_INC
    // }
}} // namespace
#   endif // v8bridge_signature_hpp
#else // BOOST_PP_IS_ITERATING
// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#   if BOOST_PP_ITERATION_DEPTH() == 1 // defined(BOOST_PP_IS_ITERATING)
#       define N BOOST_PP_ITERATION()

// as 'get_signature(RT(*)(T0...TN), void* = 0)' is the same
// function as 'get_signature(RT(__cdecl *)(T0...TN), void* = 0)',
// we don't define it twice
#       if !defined(V8_FN_CC_IS_CDECL)

template <class TResult BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TType) >
inline V8_LIST_INC(N)<TResult BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, TType) >
get_signature(TResult(V8_FN_CC *)(BOOST_PP_ENUM_PARAMS_Z(1, N, TType)), void* = 0)
{
    return V8_LIST_INC(N)<
    TResult BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, TType)
    >();
}

#       endif // !defined(V8_FN_CC_IS_CDECL)
#       undef N
#       define BOOST_PP_ITERATION_PARAMS_2 (3, (0, 3, <v8bridge/detail/signature.hpp>))

#       include BOOST_PP_ITERATE()
#   else
#       define N BOOST_PP_RELATIVE_ITERATION(1)
#       define Q V8_CV_QUALIFIER(BOOST_PP_ITERATION())

template <class TResult, class TClass BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class T) >
inline V8_LIST_INC(BOOST_PP_INC(N))<TResult, TClass& BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, T) >
get_signature(TResult(V8_FN_CC TClass::*) (BOOST_PP_ENUM_PARAMS_Z(1, N, T)) Q)
{
    return V8_LIST_INC(BOOST_PP_INC(N))
    <TResult, TClass& BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, T)>();
}


template <class TTarget, class TResult, class TClass BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class T)>
inline V8_LIST_INC(BOOST_PP_INC(N))<TResult, typename most_derived<TTarget, TClass>::type& BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, T) >
get_signature(TResult(V8_FN_CC TClass::*)(BOOST_PP_ENUM_PARAMS_Z(1, N, T)) Q, TTarget *)
{
    return V8_LIST_INC(BOOST_PP_INC(N))<TResult, BOOST_DEDUCED_TYPENAME most_derived<TTarget, TClass>::type& BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, T)>();
}

#       undef Q
#       undef N
#   endif // BOOST_PP_ITERATION_DEPTH()
#endif // !defined(BOOST_PP_IS_ITERATING)
