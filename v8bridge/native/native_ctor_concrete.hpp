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

#ifndef v8bridge_native_ctor_concrete_hpp
#define v8bridge_native_ctor_concrete_hpp

#include <v8bridge/detail/prefix.hpp>

#include <boost/mpl/if.hpp>

#include <v8bridge/conversion.hpp>
#include <v8bridge/detail/signature.hpp>
#include <v8bridge/detail/signature_formatting.hpp>
#include <v8bridge/native/native_caller.hpp>
#include <v8bridge/native/native_endpoint.hpp>
#include <v8bridge/native/native_function.hpp>
#include <v8bridge/native/native_function_concrete.hpp>
#       include <v8bridge/native/invoke_native.hpp>

#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        /**
         * This class provides a specific implementation to NativeFunctionConcrete,
         * for class constructors invocation.
         */
        template <class TFunction, class TSignature>
        class V8_DECL NativeCtorConcrete : /*public NativeFunctionConcrete<TFunction, TSignature>,*/ public NativeFunctionConcreteBase
        {
        public:
            NativeCtorConcrete(Isolate *isolationScope, TFunction function, TSignature signature)
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
                /*
                 This function is an hand-rolled version of the
                 caller.template invoke() function that sends as an argument the function callback info arg.
                 */
                
                typedef typename mpl::begin<TSignature>::type TSeqFirst;
                typedef typename TSeqFirst::type TClass;
                
                typedef typename boost::remove_reference<
                    typename boost::remove_const<
                        typename boost::remove_pointer<TClass>::type
                    >::type
                >::type *TResolvedClass;
                
                
                /* Get address by casting the External saved value.
                 Note that it was saved in NativeClass<TClass>::ctor */
                
                /* Note: We're using invoke_native_raw in function context.
                 We won't get a case that we're getting here when TFunction = method pointer
                 since we can't get the owner class instance. TFunction must be a function pointer. */
                TResolvedClass instancePtr = bridge::detail::invoke_native_raw<TResolvedClass>(
                                                                  this->m_isolationScope,
                                                                  detail::invoke_tag<TResolvedClass, TFunction>()
                                                                  , this->m_function
                                                                  , info
                                                                  );
                
                /* Save the TClass instance in the new created JS object */
                info.This()->SetAlignedPointerInInternalField(info.This()->InternalFieldCount() - 2, (void *)instancePtr);
            }
            
            /* Called when we're accepting raw native values in the function */
            template <class P>
            inline typename boost::disable_if<
            resolve_directly_passed_args<P>, void >::type
            forwardInvokeCall(const FunctionCallbackInfo<Value>& info)
            {
                typedef typename mpl::begin<TSignature>::type TSeqFirst;
                typedef typename TSeqFirst::type TClass;
                
                typedef typename boost::remove_reference<
                    typename boost::remove_const<
                        typename boost::remove_pointer<TClass>::type
                    >::type
                >::type *TResolvedClass;
                
                caller<TFunction, TSignature> resolver(this->m_isolationScope, this->m_function);
                TResolvedClass instancePtr = resolver.template invoke<TResolvedClass>(info);
                
                /* Save the TClass instance in the new created JS object */
                info.This()->SetAlignedPointerInInternalField(info.This()->InternalFieldCount() - 2, (void *)instancePtr);
            }
        };
    }
}

#endif
