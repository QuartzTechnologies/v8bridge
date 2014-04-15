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

#ifndef v8bridge_native_function_hpp
#define v8bridge_native_function_hpp

#include <boost/shared_ptr.hpp>
#include <v8bridge/detail/prefix.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>

#include <v8bridge/detail/signature.hpp>
#include <v8bridge/detail/signature_formatting.hpp>
#include <v8bridge/native/native_endpoint.hpp>
#include <v8bridge/native/native_function_concrete.hpp>

namespace v8
{
    namespace bridge
    {
        class V8_DECL NativeFunction : public NativeEndpoint
        {
        public:
            NativeFunction(Isolate *isolationScope) : NativeEndpoint(isolationScope), m_overloads(new TOverloadsList())
            {
                HandleScope handle_scope(isolationScope);
                Local<FunctionTemplate> templ = FunctionTemplate::New(
                                                                       this->m_isolationScope,
                                                                       &NativeFunction::internalFunctionInvocationCallback,
                                                                       External::New(this->m_isolationScope, this)
                                                                       );
                
                this->m_templateDecl = new Eternal<FunctionTemplate>(this->m_isolationScope, templ);
                
            }
            
            ~NativeFunction()
            {
                for (TOverloadsList::iterator it = this->m_overloads->begin(); it != this->m_overloads->end(); ++it)
                {
                    (*it).reset();
                }
                
                delete this->m_overloads;
                delete this->m_templateDecl;
            }
            
            inline Handle<FunctionTemplate> getTemplate() { return this->m_templateDecl->Get(this->m_isolationScope); }
            
            /* Standard function pointer */
            template <typename TFunction>
            inline NativeFunction *addOverload(TFunction functionPointer)
            {
                return this->addOverload(functionPointer, get_signature(functionPointer));
            }
            
            /**
             * Adds overload to the given NativeFunction JS endpoint.
             * @param functionPointer - A pointer to the function that should be executed
             * @param signature - The function boost::mpl signature. It can be received by calling get_signature(functionPointer).
             */
            template <class TFunction, class TSignature>
            inline NativeFunction *addOverload(TFunction functionPointer, TSignature signature)
            {
                NativeFunctionConcrete<TFunction, TSignature> *funcDecl = new NativeFunctionConcrete<TFunction, TSignature>(this->m_isolationScope, functionPointer, signature);
                
                boost::shared_ptr< NativeFunctionConcrete<TFunction, TSignature> > adapter(funcDecl);
                this->m_overloads->push_back(adapter);
                
                return this;
            }
            
            /**
             * Get the number of registered overloads
             */
            inline size_t getOverloadsCount() { return this->m_overloads->size(); }
            
            /**
             * Explicity invoke the given native function with the given V8 function callback info
             */
            inline void invoke(const FunctionCallbackInfo<Value>& info)
            {
                HandleScope handle_scope(info.GetIsolate());
                
                //-------------------------------------------------
                // Iterate over the registered overloads and try to find an invokable function
                //-------------------------------------------------
                
                typedef std::list<NativeFunctionConcreteBase *> TConcreteOverloads;
                TConcreteOverloads *candidates = new TConcreteOverloads();
                
                for (TOverloadsList::iterator iter = this->m_overloads->begin(); iter != this->m_overloads->end(); ++iter)
                {
                    if (iter->get()->canInvokeCall(info))
                    {
                        candidates->push_back(iter->get());
                    }
                }
                
                //std::cout << "* Candidate count: " << candidates->size() << std::endl;
                
                //-------------------------------------------------
                //  We couldn't find any method?
                //-------------------------------------------------
                
                if (candidates->size() == 0)
                {
                    std::stringstream io;
                    io << "MissingFunctionException. No overload that matches the number and/or types of provided arguments could be found." << std::endl << "Available overloads:" << std::endl;
                    for (TOverloadsList::iterator iter = this->m_overloads->begin(); iter != this->m_overloads->end(); ++iter)
                    {
                        io << "\t* " << iter->get()->getFormattedSignature() << std::endl;
                    }
                    
#if V8BRIDGE_DEBUG
                    io << std::endl << std::endl;
                    io << "-------------------------------------------------------- " << std::endl;
                    io << "Development note: Please pay attention to the available overloads above. This error may be caused because "
                        << "there's no js-to-native conversion available for your specified type. In this case, you can roll your own conversion logic "
                        << "by providing a template specification for v8::bridge::detail::NativeToJsConversion and JsToNativeConversion";
                    
                    io << std::endl << "-------------------------------------------------------- " << std::endl;
#endif
                    
                    info.GetIsolate()->ThrowException(
                                                      String::NewFromUtf8(info.GetIsolate(), io.str().c_str())
                                                      );
                    
                    delete candidates;
                    return;
                }
                
                //-------------------------------------------------
                //  We must have only ONE candidate.
                //  However, if we're dealing with direct-passed args, we
                //  could get 2 valid candidates for this call.
                //  In this case, we should fire the non-direct args function
                //-------------------------------------------------
                
                if (candidates->size() == 2)
                {
                    if ((*(candidates->begin()))->isDirectArgsFunction())
                    {
                        candidates->pop_front();
                    }
                    else if ((*(candidates->back())).isDirectArgsFunction())
                    {
                        candidates->pop_back();
                    }
                }
                
                //-------------------------------------------------
                //  Got more than one method to invoke (ambiguous call)?
                //-------------------------------------------------
                
                if (candidates->size() > 1)
                {
                    std::stringstream io;
                    io << "AmbiguousMatchException. An ambiguous function call detected for the provided arguments."
                    << std::endl << "Available candidates:" << std::endl;
                    
                    for (TConcreteOverloads::iterator iter = candidates->begin(); iter != candidates->end(); ++iter)
                    {
                        io << "\t* " << (*iter)->getFormattedSignature() << std::endl;
                    }
                    
                    info.GetIsolate()->ThrowException(
                                                      String::NewFromUtf8(info.GetIsolate(), io.str().c_str())
                                                      );
                    
                    delete candidates;
                    return;
                }
                
                //-------------------------------------------------
                //  Invoke
                //-------------------------------------------------
                
                (*(candidates->begin()))->invokeCall(info);
                
                delete candidates;
            }
        protected:
            mutable Eternal<FunctionTemplate> *m_templateDecl;
            typedef std::list<boost::shared_ptr<NativeFunctionConcreteBase> > TOverloadsList;
            
            TOverloadsList *m_overloads;
            
            /**
             * General static method used to parse incomming method invocation calls
             * and forward them to the right callback.
             *
             * @param FunctionCallbackInfo<Value> &info - The callback information sended by V8
             */
            inline static void internalFunctionInvocationCallback(const FunctionCallbackInfo<Value>& info)
            {
                //-------------------------------------------------
                //  Setup
                //-------------------------------------------------
                
                EscapableHandleScope handle_scope(info.GetIsolate());
                Handle<Context> context = info.GetIsolate()->GetCurrentContext();
                Context::Scope context_scope(context);
                
                //-------------------------------------------------
                //  Restore the calling instance
                //-------------------------------------------------
                
                NativeFunction *instance = static_cast<NativeFunction *>(External::Cast(*info.Data())->Value());
                instance->invoke(info);
            }
        };
    }
}


#endif
