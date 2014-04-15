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

#ifndef v8bridge_signature_formatting_hpp
#   define v8bridge_signature_formatting_hpp

#   include <v8bridge/detail/prefix.hpp>
#   include <v8bridge/detail/typeid.hpp>

#   include <boost/mpl/begin_end.hpp>
#   include <boost/mpl/next.hpp>
#   include <boost/mpl/size.hpp>
#   include <iostream>


namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        namespace detail
        {
            //-------------------------------------------------
            //  Generic type-to-string
            //-------------------------------------------------
            template <typename TType>
            struct TypeToString
            {
                inline std::string operator() (Isolate *isolationScope) const
                {
                    return TypeId<TType>().name();
                }
            };
            
            template <class TType>
            struct TypeToString<TType *>
            {
                inline std::string operator() (Isolate *isolationScope) const
                {
                    std::stringstream io;
                    io << TypeId<TType>().name();
                    io << " *";
                    return io.str();
                }
            };
            
            template <class TType>
            struct TypeToString<TType &>
            {
                inline std::string operator() (Isolate *isolationScope) const
                {
                    std::stringstream io;
                    io << TypeId<TType>().name();
                    io << " &";
                    return io.str();
                }
            };
            
            template <class TType>
            struct TypeToString<TType const>
            {
                inline std::string operator() (Isolate *isolationScope) const
                {
                    std::stringstream io;
                    io << TypeId<TType>().name();
                    io << " const";
                    return io.str();
                }
            };
            
            //-------------------------------------------------
            //  Simple types
            //-------------------------------------------------
            
#   define V8_TYPE_TO_STRING(type) \
                template <> \
                struct TypeToString<type> \
                { \
                    inline std::string operator() (Isolate *isolationScope) const \
                    { \
                        return "" #type ""; \
                    } \
                };
            
#   define V8_UNSIGNABLE_TYPE_TO_STRING(type) \
                V8_TYPE_TO_STRING(type) \
                V8_TYPE_TO_STRING(unsigned type)
            
            V8_UNSIGNABLE_TYPE_TO_STRING(char)
            V8_UNSIGNABLE_TYPE_TO_STRING(short)
            V8_UNSIGNABLE_TYPE_TO_STRING(int)
            V8_UNSIGNABLE_TYPE_TO_STRING(long)
            
            V8_TYPE_TO_STRING(void)
            V8_TYPE_TO_STRING(bool)
            V8_TYPE_TO_STRING(std::string)
            
            V8_TYPE_TO_STRING(v8::Handle<Value>)
            V8_TYPE_TO_STRING(v8::Handle<FunctionTemplate>)
            V8_TYPE_TO_STRING(v8::FunctionCallbackInfo<Value>)
            
#   undef V8_TYPE_TO_STRING
#   undef V8_UNSIGNABLE_TYPE_TO_STRING
            
            //-------------------------------------------------
            //  Format signature
            //-------------------------------------------------
            
            template <class End>
            inline std::string format_signature_ex(Isolate *, bool, End, End) { return ""; }
            
            template <class Iter, class End>
            inline std::string format_signature_ex(Isolate* isolationScope, bool first, Iter, End end)
            {
                std::stringstream io;
                
                if (!first)
                    io << ",";
                io << TypeToString<typename Iter::type>()(isolationScope);
                io << format_signature_ex(isolationScope, false, typename mpl::next<Iter>::type(), end);
                
                return io.str();
            }
            
        }
        
        template <class TSignature>
        inline V8_DECL std::string format_signature(Isolate *isolationScope, TSignature sign)
        {
            std::stringstream io;
            
            //-------------------------------------------------
            //  The return value
            //-------------------------------------------------
            typedef typename mpl::begin<TSignature>::type TResult;
            
            io << detail::TypeToString<typename TResult::type>()(isolationScope);
            
            io << " (";
            io << detail::format_signature_ex(
                                              isolationScope,
                                              true,
                                              typename mpl::next<TResult>::type(),
                                              typename mpl::end<TSignature>::type()
                                              );
            io << " )";
            
            return io.str();
        }
    }
}

#endif