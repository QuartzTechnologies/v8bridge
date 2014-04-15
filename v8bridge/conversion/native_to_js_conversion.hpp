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

#ifndef v8bridge_native_to_js_conversion_hpp
#define v8bridge_native_to_js_conversion_hpp

#include <v8bridge/detail/prefix.hpp>

#include <exception>
#include <string>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        namespace detail
        {
            //-------------------------------------------------
            //  Abstract
            //-------------------------------------------------
            template<typename TType>
            struct NativeToJsConversion
            {
                typedef TType resultType;
                
                inline Handle<Value> operator() (
                                                 Isolate *isolationScope,
                                                 TType value
                                                 )
                {
                    return Undefined(isolationScope);
                }
            };
            
            template<>
            struct NativeToJsConversion<void>
            {
                inline v8::Handle<v8::Value> operator() (v8::Isolate *isolationScope)
                {
                    return v8::Undefined(isolationScope);
                }
            };
            
            
            //-------------------------------------------------
            //  Boolean
            //-------------------------------------------------
            
            template<> struct NativeToJsConversion<bool>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         bool from)
                {
                    return v8::Boolean::New(isolationScope, from);
                }
            };
            
#pragma region - Integer combinations
            
            //-------------------------------------------------
            //  Integer types
            //-------------------------------------------------
            
            /* Declare int handler */
            template<typename TType>
            struct Int_NativeToJsConversion
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         TType from)
                {
                    return v8::Int32::New(isolationScope, (int)from);
                }
            };
            
            
            /* Apply it */
            
            // Short
            template <>
            struct NativeToJsConversion<short> : public Int_NativeToJsConversion<short> { };
            template <>
            struct NativeToJsConversion<unsigned short> : public Int_NativeToJsConversion<unsigned short> { };
            
            // Int
            template <>
            struct NativeToJsConversion<int> : public Int_NativeToJsConversion<int> { };
            template <>
            struct NativeToJsConversion<unsigned int> : public Int_NativeToJsConversion<unsigned int> { };
            
            // Long
            template <>
            struct NativeToJsConversion<long> : public Int_NativeToJsConversion<long> { };
            template <>
            struct NativeToJsConversion<unsigned long> : public Int_NativeToJsConversion<unsigned long> { };
            
            // Long Long
            template <>
            struct NativeToJsConversion<long long> : public Int_NativeToJsConversion<long long> { };
            template <>
            struct NativeToJsConversion<unsigned long long> : public Int_NativeToJsConversion<unsigned long long> { };
            
            //-------------------------------------------------
            //  Decimal combinations
            //-------------------------------------------------
#pragma region - Decimal combinations
            
            template<typename TType>
            struct Dec_NativeToJsConversion
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const float& from)
                {
                    return v8::Number::New(isolationScope, from);
                }
            };
            
            /* Apply */
            template <>
            struct NativeToJsConversion<double> : public Dec_NativeToJsConversion<double> { };
            
            template <>
            struct NativeToJsConversion<float> : public Dec_NativeToJsConversion<float> { };
            
            template <>
            struct NativeToJsConversion<long double> : public Dec_NativeToJsConversion<long double> { };
            
            //-------------------------------------------------
            //  String
            //-------------------------------------------------
#pragma region - std::string
            
            template<>
            struct NativeToJsConversion<std::string>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const std::string& from)
                {
                    return v8::String::NewFromUtf8(isolationScope, from.c_str());
                }
            };
            
#pragma region - const std::string
            
            template<>
            struct NativeToJsConversion<const std::string&>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const std::string& from)
                {
                    return v8::String::NewFromUtf8(isolationScope, from.c_str());
                }
            };
            
#pragma region - char *
            
            template<> struct NativeToJsConversion<char *>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const char *from)
                {
                    return v8::String::NewFromUtf8(isolationScope, from);
                }
            };
            
#pragma region - const char *
            template<>
            struct NativeToJsConversion<const char*>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const char* from)
                {
                    return v8::String::NewFromUtf8(isolationScope, from);
                }
            };
            
            //-------------------------------------------------
            //  Time
            //-------------------------------------------------
#pragma region - std::tm
            
            template<>
            struct NativeToJsConversion<std::tm>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const std::tm& from)
                {
                    std::time_t raw_time = mktime(const_cast<std::tm*>(&from));
                    
                    return v8::Date::New(isolationScope, raw_time * 1000);
                }
            };
            
            //-------------------------------------------------
            //  Vector
            //-------------------------------------------------
#pragma region - std::vector<TType>
            
            template<typename TType>
            struct NativeToJsConversion<std::vector<TType> >
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const std::vector<TType>& from)
                {
                    using namespace v8;
                    
                    Handle<Array> array = Array::New(isolationScope, (int)from.size());
                    
                    typedef typename std::vector<TType>::const_iterator iterator_type;
                    
                    int i = 0;
                    for (iterator_type it = from.begin(); it != from.end(); ++it)
                    {
                        array->Set(i++, NativeToJsConversion<TType>()(*it, isolationScope));
                    }
                    
                    return array;
                }
            };
            
            //-------------------------------------------------
            //  List
            //-------------------------------------------------
#pragma region - std::list<TType>
            
            template<typename TType>
            struct NativeToJsConversion<std::list<TType> >
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const std::list<TType>& from)
                {
                    using namespace v8;
                    
                    Handle<Array> array = Array::New(isolationScope, (int)from.size());
                    
                    typedef typename std::list<TType>::const_iterator iterator_type;
                    
                    int i = 0;
                    for (iterator_type it = from.begin(); it != from.end(); ++it)
                    {
                        array->Set(i++, NativeToJsConversion<TType >()(isolationScope, *it));
                    }
                    
                    return array;
                }
            };
            
            //-------------------------------------------------
            //  Map
            //-------------------------------------------------
#pragma region - std::map<TKey, TValue>
            
            template<typename TKey, typename TValue>
            struct NativeToJsConversion<std::map<TKey, TValue> >
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         const std::map<TKey, TValue>& from)
                {
                    using namespace v8;
                    
                    Handle<Object> object = Object::New(isolationScope);
                    
                    typedef typename std::map<TKey, TValue>::const_iterator iterator_type;
                    
                    for (iterator_type it = from.begin(); it != from.end(); ++it)
                    {
                        object->Set(
                                    NativeToJsConversion<TKey>()(isolationScope, it->first),
                                    NativeToJsConversion<TValue>()(isolationScope, it->second)
                                    );
                    }
                    
                    return object;
                }
            };
            
            //-------------------------------------------------
            //  Null
            //-------------------------------------------------
#pragma region - Null
            
            template<>
            struct NativeToJsConversion<nil>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         nil from)
                {
                    return Null(isolationScope);
                }
            };
            
            //-------------------------------------------------
            //  Undefined
            //-------------------------------------------------
#pragma region - Undefined
            
            template<>
            struct NativeToJsConversion<undefined>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         undefined from)
                {
                    return Undefined(isolationScope);
                }
            };
            
            //-------------------------------------------------
            //  Callback
            //-------------------------------------------------
#pragma region - FunctionCallback
            
            template<>
            struct NativeToJsConversion<FunctionCallback>
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         FunctionCallback from)
                {
                    return FunctionTemplate::New(isolationScope, from)->GetFunction();
                }
            };
            
            //-------------------------------------------------
            //  V8 handles
            //-------------------------------------------------
            
#pragma region - V8 Handles
            
            template<>
            struct NativeToJsConversion<Handle<Value> >
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         Handle<Value> from)
                {
                    return from; // simple forwarding
                }
            };
            
            
            template<>
            struct NativeToJsConversion<Local<Value> >
            {
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         Local<Value> from)
                {
                    EscapableHandleScope handle_scope(isolationScope);
                    return handle_scope.Escape(from); // simple forwarding
                }
            };
            
            
            //-------------------------------------------------
            //  Exceptions
            //-------------------------------------------------
#pragma region - Exceptions
            
            template <>
            struct NativeToJsConversion<std::exception>
            {
                /** Calls v8::ThrowException(ex.what()) and returns an empty
                 handle. It must call ThrowException() because that is
                 apparently the only way to create an Error object from the
                 native API.
                 */
                inline v8::Handle<v8::Value> operator() (
                                                         Isolate *isolationScope,
                                                         std::exception const & e)
                {
                    char const *msg = e.what();
                    return isolationScope->ThrowException(String::NewFromUtf8(isolationScope, msg ? msg : "[Native] Unhandled unidentified exception thrown."));
                }
            };
        }
        
        
        //-------------------------------------------------
        //  Declare the common forward API
        //
        //  Note: declaring it here since we'll need it later in this file.
        //-------------------------------------------------
        
        template <typename TType>
        inline V8_DECL v8::Handle<v8::Value> NativeToJs(Isolate *isolationScope,
                                                                    TType from)
        {
            typedef ::v8::bridge::detail::NativeToJsConversion<TType> TForwarder;
            return TForwarder()(isolationScope, from);
        }
    }
}

#endif
