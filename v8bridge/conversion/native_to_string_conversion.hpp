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

#ifndef v8bridge_native_to_string_hpp
#define v8bridge_native_to_string_hpp

#include <v8bridge/detail/prefix.hpp>

#include <v8bridge/conversion/js_to_native_conversion.hpp>
#include <v8bridge/detail/utils.hpp>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        /* Prototype */
        template <typename THandle>
        inline V8_DECL std::string NativeToString(Isolate *isolationScope,
                                                  THandle handle);
        
        namespace detail
        {
            //-------------------------------------------------
            //  Abstract
            //-------------------------------------------------
            /**
             The default implementation simply uses std::ostringstream conversion.
             */
            template<typename TType>
            struct NativeToStringConversion
            {
                template <typename TValue>
                std::string inline operator() (Isolate *isolate, TValue value) const
                {
                    std::ostringstream io;
                    io << &value;
                    return io.str();
                }
            };
            
            
            template <typename TType>
            struct NativeToStringConversion<TType *> : NativeToStringConversion<TType> {};
            
            template <typename TType>
            struct NativeToStringConversion<TType &> : NativeToStringConversion<TType> {};
            
            template <typename TType>
            struct NativeToStringConversion<const TType &> : NativeToStringConversion<TType> {};
            
            //-------------------------------------------------
            //  Void
            //-------------------------------------------------
            
            template <>
            struct NativeToStringConversion<void>
            {
                std::string inline operator() (...) const
                {
                    return "undefined";
                }
            };
            
            //-------------------------------------------------
            //  Strings
            //-------------------------------------------------
            
            template<typename TType>
            struct CString_NativeToStringConversion
            {
                inline std::string operator() (Isolate *isolate, TType value) const
                {
                    std::string s(value);
                    return escape_js_value(value);
                }
            };
            
            
            template<typename TType>
            struct String_NativeToStringConversion
            {
                inline std::string operator() (Isolate *isolate, TType value) const
                {
                    std::ostringstream io;
                    io << value;
                    return escape_js_value(value.c_str());
                }
            };
            
            /* Apply */
            template <>
            struct NativeToStringConversion<char> : CString_NativeToStringConversion<char> { };
            template <>
            struct NativeToStringConversion<char *> : CString_NativeToStringConversion<char *> { };
            template <>
            struct NativeToStringConversion<const char *> : CString_NativeToStringConversion<const char *> { };
            template <>
            struct NativeToStringConversion<std::string> : String_NativeToStringConversion<std::string> { };
            
            //-------------------------------------------------
            //  Boolean
            //-------------------------------------------------
            template <>
            struct NativeToStringConversion<bool>
            {
                inline std::string operator() (Isolate *isolate, bool value) const
                {
                    return value ? "true" : "false";
                }
            };
            
            
            //-------------------------------------------------
            //  V8 Objects
            //-------------------------------------------------
            
            template <>
            struct NativeToStringConversion<Object>
            {
                int m_iterationCounter = 0;
                const int MAX_ITER_COUNT = 20; /* Recursion protection */
                
                inline std::string operator()(Isolate *isolationScope, Object *value) const
                {
                    if (m_iterationCounter > MAX_ITER_COUNT)
                    {
                        return "... RECURSION ...";
                    }
                    HandleScope handle_scope(isolationScope);
                    
                    if (value == NULL)
                    {
                        return "null";
                    }
                    
                    
                    
                    std::ostringstream io;
                    const bool isArray = value->IsArray();
                    const char *separator = isArray ? "[]" : "{}";
                    
                    io << separator;
                    
                    Local<Array> ar(value->GetPropertyNames());
                    uint32_t len = ar->Length();
                    for( uint32_t i = 0; i < len; ++i )
                    {
                        if ( isArray )
                        {
                            io <<  NativeToString(isolationScope, value->Get(Int32::New(isolationScope, i)));
                        }
                        else
                        {
                            std::string tmp;
                            
                            Local<Value> key(ar->Get(Int32::New(isolationScope, i)));
                            JsToNativeConversion<std::string>()(isolationScope, tmp, key);
                            io << tmp;
                            
                            io << ":";
                            
                            io << NativeToString(isolationScope, value->Get(key));
                        }
                        
                        if ((i + 1) < len)
                        {
                            io << ", ";
                        }
                    }
                    
                    io << separator;
                    return io.str();
                }
                
                
                template <typename THandle>
                inline std::string operator()(Isolate *isolationScope, THandle h) const
                {
                    return h.IsEmpty() ? "undefined" : this->operator() ( Object::Cast(h) );
                }
            };
            
            //-------------------------------------------------
            //  Arrays
            //-------------------------------------------------
            
            template <>
            struct NativeToStringConversion<Array>
            {
                std::string operator()(Isolate *isolationScope, Array *value) const
                {
                    if (value == NULL)
                    {
                        return "null";
                    }
                    
                    std::ostringstream io;
                    io << '[';
                    uint32_t len = value->Length();
                    
                    for (uint32_t i = 0; i < len; ++i)
                    {
                        io <<  NativeToString(isolationScope, value->Get(Int32::New(isolationScope, i)));
                        if ((i + 1) < len)
                        {
                            io << ", ";
                        }
                    }
                    
                    io << ']';
                    return io.str();
                }
                
                template <typename THandle>
                inline std::string operator() (Isolate *isolationScope, THandle h) const
                {
                    return h.IsEmpty() ? "undefined" : this->operator() ( Array::Cast(h) );
                }
            };
            
            //-------------------------------------------------
            //  Functions
            //-------------------------------------------------
            template <>
            struct NativeToStringConversion<Function>
            {
                inline std::string operator() (Isolate *isolationScope, Function *value) const
                {
                    if (value == NULL)
                    {
                        return "null";
                    }
                    
                    std::string fName;
                    JsToNative<std::string>( isolationScope, fName, value->GetName() );
                    
                    if ( fName.empty() )
                    {
                        std::string ret;
                        JsToNative<std::string>( isolationScope, ret, value->ToString() );
                        return ret;
                    }
                    
                    return fName;
                }
                
                template <typename THandle>
                inline std::string operator() (Isolate *isolationScope, THandle h) const
                {
                    return h.IsEmpty() ? "undefined" : this->operator() ( Array::Cast(h) );
                }
            };
            
            //-------------------------------------------------
            //  Handles
            //-------------------------------------------------
            
            template <class THandle>
            struct Handle_NativeToStringConversion
            {
                inline std::string operator() (Isolate *isolationScope, THandle h) const
                {
                    if ( h.IsEmpty() || h->IsUndefined() )
                    {
                        return "undefined";
                    }
                    else if ( h->IsNull() )
                    {
                        return "null";
                    }
                    else if ( h->IsString() )
                    {
                        std::string tmp;
                        JsToNative<std::string>(isolationScope, tmp, h);
                        return NativeToString(isolationScope, tmp);
                    }
                    else if ( h->IsBoolean() )
                    {
                        return NativeToString(isolationScope, h->BooleanValue());
                    }
                    else if ( h->IsInt32() )
                    {
                        int tmp;
                        JsToNative<int>(isolationScope, tmp, h);
                        return NativeToString(isolationScope, tmp);
                    }
                    else if ( h->IsNumber() )
                    {
                        double tmp;
                        JsToNative<double>(isolationScope, tmp, h);
                        return NativeToString(isolationScope, tmp);
                    }
                    else if ( h->IsFunction() )
                    {
                        return NativeToString(isolationScope, Function::Cast(*h));
                    }
                    else if ( h->IsArray() )
                    {
                        return NativeToString(isolationScope, Array::Cast(*h));
                    }
                    else if ( h->IsObject() )
                    {
                        return NativeToString(isolationScope, Object::Cast(*h));
                    }
                    
                    return "[Native] Casting Error: NativeToStringConversion<Handle<T> > recived un-maped type!";
                }
            };
            
            /* Apply */
            template <typename T>
            struct NativeToStringConversion<Handle<T> > : Handle_NativeToStringConversion<Handle<T> > { };
            
            template <typename T>
            struct NativeToStringConversion<Local<T> > : Handle_NativeToStringConversion<Local<T> > { };
            
            template <typename T>
            struct NativeToStringConversion<Persistent<T> > : Handle_NativeToStringConversion<Persistent<T> > { };
            
            //-------------------------------------------------
            //  Eternal
            //-------------------------------------------------
            
            template <class T>
            struct NativeToStringConversion<Eternal<T> >
            {
                inline std::string operator() (Isolate *isolationScope, Eternal<T> value) const
                {
                    return NativeToString(isolationScope, value->Get());
                }
            };
            
            //-------------------------------------------------
            //  Lists and vectors
            //-------------------------------------------------
            template <typename TSeq>
            struct Seq_NativeToStringConversion
            {
                typedef TSeq TType;
                
                inline std::string operator() (Isolate *isolationScope, TSeq src) const
                {
                    if ( src.empty() )
                    {
                        return "({})";
                    }
                    
                    std::ostringstream os;
                    os << "[";
                    size_t sz = src.size();
                    
                    typename TType::const_iterator it, et;
                    it = src.begin();
                    et = src.end();
                    size_t i = 0;
                    for( ; et != it; ++it )
                    {
                        os << NativeToString(isolationScope, (*it));
                        if ( ++i != sz ) os << ", ";
                    }
                    
                    os << "]\n";
                    return os.str();
                }
            };
            
            /* Apply. */
            template <typename TType>
            struct NativeToStringConversion<std::list<TType> > : Seq_NativeToStringConversion<std::list<TType> > {};
            
            template <typename TType>
            struct NativeToStringConversion<std::vector<TType> > : Seq_NativeToStringConversion<std::vector<TType> > {};
        }
        
        
        template <typename THandle>
        inline V8_DECL std::string NativeToString(Isolate *isolationScope,
                                       THandle handle)
        {
            typedef detail::NativeToStringConversion<THandle> TForwarder;
            return TForwarder()(isolationScope, handle);
        }
    }
}

#endif
