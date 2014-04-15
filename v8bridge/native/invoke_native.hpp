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
 *  This file declares an interface to invoke native functions from the supplied function arguments.
 *
 *  The following macros generate expansions for::
 *
 *      Accepts function pointer and returns the function value converted to V8 JS value (using NativeToJs) (invoke_tag_aux<false, false>)
 *          template <class TCallback, TArg0 ... TArgN>
 *          inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<false, false>, TCallback& callback, TArg0 ... TArgN)
 *          {
 *                  return NativeToJs(isolationScope,
 *                      callback( TArg0 ... TArgN )
 *                  );
 *          }
 *
 *      Accepts function pointer and returns void (invoke_tag_aux<true, false>)
 *          template <class TCallback, TArg0 ... TArgN>
 *          inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<true, false>, TCallback& callback, TArg0 ... TArgN)
 *          {
 *                  callback( TArg0 ... TArgN );
 *                  return v8::Undefined(isolationScope);
 *          }
 *
 *      Accepts class method pointer and returns the method value converted to V8 JS value (using NativeToJs) (invoke_tag_aux<true, true>)
 *          template <class TCallback, class TClass, TArg0 ... TArgN>
 *          inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<true, true>,
 *                       TCallback& callback, TClass& instance, TArg0 ... TArgN)
 *          {
 *                 return NativeToJs(isolationScope,
 *                          (instance->*callback)( TArg0 ... TArgN )
 *                      );
 *          }
 *
 *      Accepts class method pointer and returns void (invoke_tag_aux<false, true>)
 *          template <class TCallback, class TClass, TArg0 ... TArgN>
 *          inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<true, true>,
 *                       TCallback& callback, TClass& instance, TArg0 ... TArgN)
 *          {
 *                  (instance->*callback)( TArg0 ... TArgN );
 *                  return v8::Undefined(isolationScope);
 *          }
 *
 * In addition, the file also generates two "_aux" suffix'd functions, which allows to directly receive the invoked function returned value.
 *
 *      Accept function and return its value:
 *          template <class class TResult, TCallback, TArg0 ... TArgN>
 *          inline static TResult invoke_native(Isolate *isolationScope, invoke_tag_aux<false, false>,
 *                       TCallback& callback, TArg0 ... TArgN)
 *          {
 *                  return (instance->*callback)( TArg0 ... TArgN );
 *          }
 *
 *          template <class class TResult, TCallback, class TClass, TArg0 ... TArgN>
 *          inline static TResult invoke_native(Isolate *isolationScope, invoke_tag_aux<true, true>,
 *                       TCallback& callback, TClass& instance, TArg0 ... TArgN)
 *          {
 *                  return (instance->*callback)( TArg0 ... TArgN );
 *          }
 */

#ifndef BOOST_PP_IS_ITERATING
#   ifndef v8bridge_invoke_native_hpp
#       define v8bridge_invoke_native_hpp
#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/native/invoke_v8_handle.hpp>

#       include <boost/preprocessor/repetition.hpp>
#       include <boost/preprocessor/iteration/iterate.hpp>
#       include <boost/type_traits/is_member_function_pointer.hpp>

#       include <v8/v8.h>

namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            using namespace v8;
            
            template <bool is_return_void, bool is_class_method>
            struct invoke_tag_aux {};
            
            // A metafunction returning the appropriate tag type for invoking an
            // object of type F with return type R.
            template <class TResult, class TCallback>
            struct invoke_tag : invoke_tag_aux <
            boost::is_same<TResult, void>::value
            , boost::is_member_function_pointer<TCallback>::value
            > { };
            
#       define BOOST_PP_ITERATION_PARAMS_1 (3, (0, V8_MAX_ARITY, <v8bridge/native/invoke_native.hpp>))
#       include BOOST_PP_ITERATE()
        }
    }
}

#   endif // v8bridge_invoke_native_hpp
#else // BOOST_PP_IS_ITERATING
#   define N BOOST_PP_ITERATION()

/* Iterations of Function that Return Value */
template <class TCallback BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TArg)>
inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<false, false>, TCallback& callback BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(1, N, TArg, & arg) )
{
    return NativeToJs(isolationScope,
                      callback( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, arg, BOOST_PP_INTERCEPT) )
                      );
}

template <class TResult, class TCallback BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TArg)>
inline static TResult invoke_native_raw(Isolate *isolationScope, invoke_tag_aux<false, false>, TCallback& callback BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(1, N, TArg, & arg) )
{
    return callback( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, arg, BOOST_PP_INTERCEPT) );
}

/* Iterations of Function that Return Void */
template <class TCallback BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TArg)>
inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<true, false>, TCallback& callback BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(1, N, TArg, & arg) )
{
    callback( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, arg, BOOST_PP_INTERCEPT) );
    return Undefined(isolationScope);
}

/* Iterations of Method that Return Value */
template <class TCallback, class TClass BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TArg)>
inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<false, true>, TCallback& callback, TClass& instance BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(1, N, TArg, & arg) )
{
    return NativeToJs(isolationScope,
                      (instance->*callback)( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, arg, BOOST_PP_INTERCEPT) )
                      );
}

template <class TResult, class TCallback, class TClass BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TArg)>
inline static TResult invoke_native_raw(Isolate *isolationScope, invoke_tag_aux<false, true>, TCallback& callback, TClass& instance BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(1, N, TArg, & arg) )
{
    return (instance->*callback)( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, arg, BOOST_PP_INTERCEPT) );
}

/* Iterations of Method that Return Void */
template <class TCallback, class TClass BOOST_PP_ENUM_TRAILING_PARAMS_Z(1, N, class TArg)>
inline static Handle<Value> invoke_native(Isolate *isolationScope, invoke_tag_aux<true, true>, TCallback& callback, TClass& instance BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(1, N, TArg, & arg) )
{
    (instance->*callback)( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, arg, BOOST_PP_INTERCEPT) );
    return Undefined(isolationScope);
}


#endif
