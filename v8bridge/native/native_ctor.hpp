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

#ifndef v8bridge_native_ctor_hpp
#define v8bridge_native_ctor_hpp

#include <boost/shared_ptr.hpp>
#include <v8bridge/detail/prefix.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>

#include <v8bridge/detail/signature.hpp>
#include <v8bridge/detail/signature_formatting.hpp>
#include <v8bridge/native/native_endpoint.hpp>
#include <v8bridge/native/native_function.hpp>
#include <v8bridge/native/native_function_concrete.hpp>
#include <v8bridge/native/native_ctor_concrete.hpp>

namespace v8
{
    namespace bridge
    {
        /**
         * This is a specific NativeFunction implementation
         * that is been used to register class constructor overloads (using ctor, see ctor.hpp)
         */
        class V8_DECL NativeCtor : public NativeFunction
        {
        public:
            NativeCtor(Isolate *isolationScope) : NativeFunction(isolationScope)
            {
            }
            
            template <class TFunction, class TSignature>
            inline NativeFunction *addOverload(TFunction functionPointer, TSignature signature)
            {
                /* Instead of registering NativeFunctionConcrete, we should register NativeCtorConcrete */
                NativeCtorConcrete<TFunction, TSignature> *ctorDecl = new NativeCtorConcrete<TFunction, TSignature>(this->m_isolationScope, functionPointer, signature);
                
                boost::shared_ptr< NativeCtorConcrete<TFunction, TSignature> > adapter(ctorDecl);
                this->m_overloads->push_back(adapter);
                
                return this;
            }
        };
    }
}

#endif
