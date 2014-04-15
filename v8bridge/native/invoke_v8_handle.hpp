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
 *  This file declares an interface to invoke v8 function handles (Handle<Function>) from the supplied C++ native function arguments.
 *
 *  The following macros generate expansions for::
 *
 *      template <class TClass0 ... TClassN>
 *      static inline Local<Value> invoke_v8_handle(Isolate *isolationScope, Handle<Function> &callback, TClass0 arg0 ... TClassN argN) {
 *          Handle<Value> argv[] = {
 *              arg0 ... argN
 *          };
 *          return callback->Call(callback, N, argv);
 *      }
 */

#ifndef BOOST_PP_IS_ITERATING
#   ifndef v8bridge_invoke_v8_handle_hpp
#       define v8bridge_invoke_v8_handle_hpp

#       include <stdexcept>

#       include <v8bridge/detail/prefix.hpp>

#       include <v8bridge/conversion.hpp>   // Include the conversion API

#       include <boost/preprocessor/repetition.hpp>
#       include <boost/preprocessor/iteration/iterate.hpp>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        /**
         * Base specification for invoking V8 function.
         * @param Isolate isolationScope - the used isolation scope
         * @param Handle<Function> func - the function to invoke
         * @return Local<Handle> the call returned value
         */
        static inline Local<Value> invoke_v8_handle_raw(Isolate *isolationScope, Handle<Function> &callback) {
            Handle<Value> argv[] = {};
            return callback->Call(callback, 0, argv);
        }
        
        /**
         * Base specification for invoking V8 function (with type conversion).
         * @param Isolate isolationScope - the used isolation scope
         * @param Handle<Function> func - the function to invoke
         * @return Local<Handle> the call returned value
         */
        template <class TResult>
        static inline TResult invoke_v8_handle(Isolate *isolationScope, Handle<Function> &callback) {
            Handle<Value> argv[] = {};
            TResult result;
            JsToNative(isolationScope, result, callback->Call(callback, 0, argv));
            return result;
        }
        
#       define BOOST_PP_ITERATION_PARAMS_1 (3, (1, V8_MAX_ARITY, <v8bridge/native/invoke_v8_handle.hpp>))
#       include BOOST_PP_ITERATE()
    }
}

#   endif // v8bridge_invoke_v8_handle_hpp
#else // BOOST_PP_IS_ITERATING
#   if BOOST_PP_ITERATION_DEPTH() == 1 // defined(BOOST_PP_IS_ITERATING)
#       define N BOOST_PP_ITERATION()
#       define V8_BRIDGE_CALL_CONCAT_ARG(z, n, data)                    BOOST_PP_CAT(TClass, n) BOOST_PP_CAT(arg, n)
#       define V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE(z, n, data)        NativeToJs<BOOST_PP_CAT(TClass, n)>(isolationScope, BOOST_PP_CAT(arg, n))

template <BOOST_PP_ENUM_PARAMS(N, class TClass) >
static inline Local<Value> invoke_v8_handle_raw(Isolate *isolationScope, Handle<Function> &callback, BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~)) {
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    return callback->Call(callback, N, argv);
}

template <class TResult, BOOST_PP_ENUM_PARAMS(N, class TClass) >
static inline TResult invoke_v8_handle(Isolate *isolationScope, Handle<Function> &callback, BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~)) {
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    TResult result;
    if (!JsToNative<TResult>(isolationScope, result, callback->Call(callback, N, argv)))
    {
#if V8BRIDGE_DEBUG
        throw std::runtime_error("The provided TResult type does not match the returned value or was not registered with the Conversion API.");
#else
        return v8::Undefined(isolationScope);
#endif
    }
    
    return result;
}

#       undef V8_BRIDGE_CALL_CONCAT_ARG
#       undef V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE
#   endif //BOOST_PP_ITERATION_DEPTH
#endif
