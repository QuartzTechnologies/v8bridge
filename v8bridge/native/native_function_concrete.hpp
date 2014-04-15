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
 * This files contains implementations for concrete native function.
 * We're using it in order to resolve and invoke specific overloads of a given native function.
 *
 * For example - in case we wish to expose the given two functions (overloaded):
 *      int multiply(int a) { return a * 2; }
 *      int multiply(int a, int b) { return a * b; }
 *
 * NativeFunction, which is the native function container class, will allows you to add these two overloads.
 * each overload, will have a NativeFunctionConcrete object associated with it.
 * The NativeFunctionConcrete will tells NativeFunction what's the function arity, if the passed parameters match its signature (type check) and allow to invoke the function.
 */

#ifndef v8bridge_native_function_concrete_hpp
#define v8bridge_native_function_concrete_hpp

#include <v8bridge/detail/prefix.hpp>

#include <boost/mpl/if.hpp>


#include <v8bridge/conversion.hpp>
#include <v8bridge/detail/signature.hpp>
#include <v8bridge/detail/signature_formatting.hpp>
#include <v8bridge/native/native_endpoint.hpp>
#include <v8bridge/native/native_caller.hpp>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        //-------------------------------------------------
        //  Native function implementation interface
        //-------------------------------------------------
        
        /* Basic abstraction class. We need this class to store the NativeFunctionConcrete without knowing its "TFunction" and "TSignature" 
            in addition to save different NativeFunctionConcretes (which will have diffrent TFunction and/or TSignature) in the same list. */
        class V8_DECL NativeFunctionConcreteBase : public NativeEndpoint
        {
        public:
            NativeFunctionConcreteBase(Isolate *isolationScope)
            : NativeEndpoint(isolationScope) { }
            
            virtual bool canInvokeCall(const FunctionCallbackInfo<Value>& info) = 0;
            virtual void invokeCall(const FunctionCallbackInfo<Value>& info) = 0;
            virtual std::string getFormattedSignature() = 0;
            virtual bool isDirectArgsFunction() = 0;
        };
        
        //-------------------------------------------------
        //  Generic implementation
        //-------------------------------------------------
        template <class TFunction, class TSignature>
        class V8_DECL NativeFunctionConcrete : public NativeFunctionConcreteBase
        {
        public:
            NativeFunctionConcrete(Isolate *isolationScope, TFunction function, TSignature signature)
            : NativeFunctionConcreteBase(isolationScope), m_function(function), m_signature(signature)
            {
                
            }
            
            inline bool canInvokeCall(const FunctionCallbackInfo<Value>& info)
            {
                return this->forwardCanInvokeCall<TSignature>(info);
            }
            
            inline void invokeCall(const FunctionCallbackInfo<Value>& info)
            {
                this->forwardInvokeCall<TSignature>(info);
            }
            
            inline std::string getFormattedSignature()
            {
                return format_signature(this->m_isolationScope, this->m_signature);
            }
            
            inline bool isDirectArgsFunction()
            {
                return resolve_directly_passed_args<TSignature>::value;
            }
        protected:
            TFunction m_function;
            TSignature m_signature;
            
            /* Called when we're dealing with function that accepts the V8 args directly */
            template <class P>
            inline typename boost::enable_if<
            resolve_directly_passed_args<P>, bool >::type
            forwardCanInvokeCall(const FunctionCallbackInfo<Value>& info)
            {
                return true; // Always true
            }
            
            /* Called when we're accepting raw native values in the function */
            template <class P>
            inline typename boost::disable_if<
            resolve_directly_passed_args<P>, bool >::type
            forwardCanInvokeCall(const FunctionCallbackInfo<Value>& info)
            {
                caller<TFunction, TSignature> resolver(this->m_isolationScope, this->m_function);
                return info.Length() == resolver.getArity() && resolver.isInvokable(info);
            }
            
            
            /* Called when we're dealing with function that accepts the V8 args directly */
            template <class P>
            inline typename boost::enable_if<
            resolve_directly_passed_args<P>, void >::type
            forwardInvokeCall(const FunctionCallbackInfo<Value>& info)
            {
                this->forwardDirectArgsInvoke<TSignature>(info);
            }
            
            /* Called when we're accepting raw native values in the function */
            template <class P>
            inline typename boost::disable_if<
            resolve_directly_passed_args<P>, void >::type
            forwardInvokeCall(const FunctionCallbackInfo<Value>& info)
            {
                caller<TFunction, TSignature> resolver(this->m_isolationScope, this->m_function);
                info.GetReturnValue().Set(resolver(info));
            }
            
            /* Invoked in case we're dealing with direct-args function that return void */
            template <class P>
            inline typename boost::enable_if<
            boost::is_same<void, typename boost::mpl::at_c<P, 0>::type >, void >::type
            forwardDirectArgsInvoke(const FunctionCallbackInfo<Value>& info) {
                (this->m_function)(info);
                //  The function should set the return value if she wants herself.
                //  Otherwise, V8 sets the default value as Undefined.
                //return Undefined(info.GetIsolate());
            }
            
            /* Invoked in case we're dealing with direct-args function that return custom type */
            template <class P>
            inline typename boost::disable_if<
            boost::is_same<void, typename boost::mpl::at_c<P, 0>::type >, void >::type
            forwardDirectArgsInvoke(const FunctionCallbackInfo<Value>& info) {
                Handle<Value> result =  NativeToJs(
                                                   info.GetIsolate(),
                                                   (this->m_function)(info)
                                                   );
                info.GetReturnValue().Set(
                                          result
                                          );
            }
            
        };
    }
}

#endif
