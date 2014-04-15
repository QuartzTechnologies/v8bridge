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

#ifndef v8bridge_js_to_native_conversion_hpp
#define v8bridge_js_to_native_conversion_hpp

#include <v8bridge/detail/prefix.hpp>

#include <v8bridge/conversion.hpp>
#include <v8bridge/conversion/type_resolver.hpp>
#include <v8bridge/detail/typeid.hpp>
#include <v8bridge/primitive.hpp>
#include <list>
#include <map>
#include <vector>
#include <stdlib.h>
#include <stddef.h>


namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            //-------------------------------------------------
            //  Generic delaration
            //-------------------------------------------------
            template<typename TType>
            struct JsToNativeConversion
            {
                typedef TType resultType;

                inline bool operator() (
                                        Isolate *isolationScope,
                                        TType& to,
                                        Handle<Value> from)
                {
                    return false;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from) { return false; }
            };

#pragma region - std::tm

            //-------------------------------------------------
            //  Time
            //-------------------------------------------------
            template<>
            struct JsToNativeConversion<std::tm>
            {
                inline bool operator()(
                                       Isolate *isolationScope,
                                       std::tm& to,
                                       v8::Handle<v8::Value> from)
                {
                    if (!from->IsDate() && !from->IsString())
                    {
                        return false;
                    }

                    v8::Handle<v8::Date> date = v8::Handle<v8::Date>::Cast(from);

                    time_t rawTime = date->NumberValue()/1000;
                    to = *localtime(&rawTime);

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsDate() || !from->IsString();
                }
            };

#pragma region - Int combinations

            //-------------------------------------------------
            //  Int combinations
            //-------------------------------------------------

            /* Generic conversion structure */
            template <typename TIntType>
            struct Int_JsToNativeConversion
            {
                inline bool operator()(
                                       Isolate *isolationScope,
                                       TIntType& to,
                                       v8::Handle<v8::Value> from)
                {
                    if (!from->IsInt32())
                    {
                        return false;
                    }

                    to = from->Int32Value();

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsInt32();
                }
            };

            /* Apply it */

            // Short
            template <>
            struct JsToNativeConversion<short> : public Int_JsToNativeConversion<short> { };
            template <>
            struct JsToNativeConversion<unsigned short> : public Int_JsToNativeConversion<unsigned short> { };

            // Int
            template <>
            struct JsToNativeConversion<int> : public Int_JsToNativeConversion<int> { };
            template <>
            struct JsToNativeConversion<unsigned int> : public Int_JsToNativeConversion<unsigned int> { };

            // Long
            template <>
            struct JsToNativeConversion<long> : public Int_JsToNativeConversion<long> { };
            template <>
            struct JsToNativeConversion<unsigned long> : public Int_JsToNativeConversion<unsigned long> { };

            // Long Long
            template <>
            struct JsToNativeConversion<long long> : public Int_JsToNativeConversion<long long> { };
            template <>
            struct JsToNativeConversion<unsigned long long> : public Int_JsToNativeConversion<unsigned long long> { };

#pragma region - Decimal numbers combinations

            //-------------------------------------------------
            //  Decimal numbers
            //-------------------------------------------------

            /* Generic conversion structure */
            template <typename TDecimalType>
            struct Dec_JsToNativeConversion
            {
                inline bool operator()(
                                       Isolate *isolationScope,
                                       TDecimalType& to,
                                       v8::Handle<v8::Value> from)
                {
                    if (!from->IsNumber() && !from->IsNumberObject())
                    {
                        return false;
                    }

                    to = (TDecimalType)from->NumberValue();

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsNumber() || from->IsNumberObject();
                }
            };

            /* Apply it, NOW! */

            //  Double
            template <>
            struct JsToNativeConversion<double> : public Dec_JsToNativeConversion<double> { };
            template <>
            struct JsToNativeConversion<long double> : public Dec_JsToNativeConversion<long double> { };

            // Float
            template <>
            struct JsToNativeConversion<float> : public Dec_JsToNativeConversion<float> { };


#pragma region - bool

            //-------------------------------------------------
            //  Boolean
            //-------------------------------------------------
            template<>
            struct JsToNativeConversion<bool>
            {
                inline bool operator()(
                                       Isolate *isolationScope,
                                       bool& to,
                                       v8::Handle<v8::Value> from)
                {
                    if (!from->IsBoolean() && !from->IsBooleanObject())
                    {
                        return false;
                    }

                    to = from->BooleanValue();

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsBoolean() || from->IsBooleanObject();
                }
            };

#pragma region - char

            template<>
            struct JsToNativeConversion<char>
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        char &to,
                                        v8::Handle<v8::Value> from)
                {
                    if (!from->IsString() && !from->IsStringObject())
                    {
                        return false;
                    }

                    v8::String::Utf8Value utf8_value(from);
                    to = **utf8_value; // first letter...

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsString() || from->IsStringObject();
                }
            };

#pragma region - string combinations
            //-------------------------------------------------
            //  String combinations
            //-------------------------------------------------

            /* Declare */
            template<typename TStringType>
            struct Str_JsToNativeConversion
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        TStringType& to,
                                        v8::Handle<v8::Value> from)
                {
                    if (!from->IsString() && !from->IsStringObject())
                    {
                        return false;
                    }

                    v8::String::Utf8Value utf8_value(from);
                    to = *utf8_value;

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsString() || from->IsStringObject();
                }
            };

            /* Apply */

            // char *
            template <>
            struct JsToNativeConversion<char *> : public Str_JsToNativeConversion<char *> { };
            template <>
            struct JsToNativeConversion<const char *> : public Str_JsToNativeConversion<const char *> { };

            // std::string
            template <>
            struct JsToNativeConversion<std::string> : public Str_JsToNativeConversion<std::string> { };
            template <>
            struct JsToNativeConversion<std::string &> : public Str_JsToNativeConversion<std::string &> { };


#pragma region - Pointer
            //-------------------------------------------------
            //  Pointer
            //-------------------------------------------------

            template<typename T>
            struct JsToNativeConversion<T *>
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        T*& to,
                                        v8::Handle<v8::Value> from)
                {
                    if (from.IsEmpty())
                    {
                        return false;
                    }

                    if (from->IsNull())
                    {
                        to = 0;
                        return true;
                    }

                    if (!from->IsObject())
                    {
                        return false;
                    }

                    v8::Handle<v8::Object> object = v8::Handle<v8::Object>::Cast(from);

                    if (object.IsEmpty())
                    {
                        return false;
                    }

                    void *ptr = object->GetAlignedPointerFromInternalField(object->InternalFieldCount() - 2);
                    
                    if (!ptr)
                    {
                        return false;
                    }

                    to = static_cast<T*>(ptr);

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return !from->IsNull() && !from.IsEmpty() && from->IsObject();
                }
            };
            
#pragma region - Null
            //-------------------------------------------------
            // Null
            //-------------------------------------------------
            
            template<>
            struct JsToNativeConversion<nil>
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        nil &to,
                                        v8::Handle<v8::Value> from)
                {
                    if (!from->IsNull())
                    {
                        return false;
                    }
                    
                    to = nil();
                    return true;
                }
                
                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsNull();
                }
            };
            
            
#pragma region - Undefined
            //-------------------------------------------------
            // Undefined
            //-------------------------------------------------
            
            template<>
            struct JsToNativeConversion<undefined>
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        undefined &to,
                                        v8::Handle<v8::Value> from)
                {
                    if (!from->IsUndefined())
                    {
                        return false;
                    }
                    
                    to = undefined();
                    return true;
                }
                
                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsUndefined();
                }
            };
            
#pragma region - Generic v8 Handle

            //-------------------------------------------------
            //  Handle<Value>
            //-------------------------------------------------

            template<>
            struct JsToNativeConversion<v8::Handle<v8::Value > >
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        v8::Handle<Value> &to,
                                        v8::Handle<v8::Value> from)
                {
                    to = from; // simple copy
                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return true;
                }
            };

#pragma region - V8 handles type-specific

#define V8_HANDLE_TYPE_SPECIFIC_CONVERSION(type) \
            template <> \
            struct JsToNativeConversion<v8::Handle<type > > \
            { \
                inline bool operator() ( \
                                        Isolate *isolationScope, \
                                        v8::Handle<type> &to, \
                                        v8::Handle<v8::Value> from) \
                { \
                    if (!from->Is##type()) \
                    { \
                        return false; \
                    } \
                    to = Handle<type>::Cast(static_cast<Handle<Value > >(from)); \
                    return true; \
                } \
                \
                inline bool operator() ( \
                                        Isolate *isolationScope, \
                                        v8::Handle<type> &to, \
                                        v8::Local<v8::Value> from) \
                { \
                    EscapableHandleScope handle_scope(isolationScope); \
                    if (!from->Is##type()) \
                    { \
                        return false; \
                    } \
                    Local<type> result = Local<type>::Cast(static_cast<Local<Value > >(from)); \
                    to = handle_scope.Escape(result); \
                    return true; \
                } \
                \
                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from) \
                { \
                    return from->Is##type(); \
                } \
            }
            
            /* String */
            V8_HANDLE_TYPE_SPECIFIC_CONVERSION(String);
            
            /* Number */
            V8_HANDLE_TYPE_SPECIFIC_CONVERSION(Number);
            
            /* Date */
            V8_HANDLE_TYPE_SPECIFIC_CONVERSION(Date);
            
            /* Array: */
            V8_HANDLE_TYPE_SPECIFIC_CONVERSION(Array);

            /* Object: */
            V8_HANDLE_TYPE_SPECIFIC_CONVERSION(Object);

            /* Function: */
            V8_HANDLE_TYPE_SPECIFIC_CONVERSION(Function);
            
#undef V8_HANDLE_TYPE_SPECIFIC_CONVERSION

            //-------------------------------------------------
            //  Some types can't use the macro above, so complete them by hand
            //  Yes... its hacky and dirtry, but WHO CARE?!
            //-------------------------------------------------
            
            /* Integer */
            template <>
            struct JsToNativeConversion<v8::Handle<Integer > >
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        v8::Handle<Integer> &to,
                                        v8::Handle<v8::Value> from)
                {
                    if (!from->IsInt32())
                    {
                        return false;
                    }
                    to = Handle<Integer>::Cast(static_cast<Handle<Value > >(from));
                    return true;
                }
                
                inline bool operator() (
                                        Isolate *isolationScope,
                                        v8::Handle<Integer> &to,
                                        v8::Local<v8::Value> from)
                {
                    EscapableHandleScope handle_scope(isolationScope);
                    if (!from->IsInt32())
                    {
                        return false;
                    }
                    Local<Integer> result = Local<Integer>::Cast(static_cast<Local<Value > >(from));
                    to = handle_scope.Escape(result);
                    return true;
                }
                
                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsInt32();
                }
            };
            
            /* Boolean */
            template <>
            struct JsToNativeConversion<v8::Handle<Boolean > >
            {
                inline bool operator() (
                                        Isolate *isolationScope,
                                        v8::Handle<Boolean> &to,
                                        v8::Handle<v8::Value> from)
                {
                    if (!from->IsBoolean())
                    {
                        return false;
                    }
                    
                    /* Since we can't cast directly Handle<Value> to Handle<Boolean>,
                        and we got two pre-defined options (true/false) - we can simply hack through it. */
                    EscapableHandleScope handle_scope(isolationScope);
                    Local<Boolean> value = Boolean::New(isolationScope, from->IsTrue() ? true : false);
                    to = handle_scope.Escape(value);
                    return true;
                }
                
                inline bool operator() (
                                        Isolate *isolationScope,
                                        v8::Handle<Boolean> &to,
                                        v8::Local<v8::Value> from)
                {
                    if (!from->IsBoolean())
                    {
                        return false;
                    }
                    
                    /* Since we can't cast directly Handle<Value> to Handle<Boolean>,
                     and we got two pre-defined options (true/false) - we can simply hack through it. */
                    EscapableHandleScope handle_scope(isolationScope);
                    Local<Boolean> value = Boolean::New(isolationScope, from->IsTrue() ? true : false);
                    to = handle_scope.Escape(value);
                    
                    return true;
                }
                
                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from->IsBoolean();
                }
            };
            

#pragma region - General list convertor
            //-------------------------------------------------
            //  Sequence
            //-------------------------------------------------

            template<typename TType, typename TElemType>
            struct Seq_JsToNativeConversion
            {
                inline bool operator()(
                                       Isolate *isolationScope,
                                       TType& to,
                                       v8::Handle<v8::Value> from)
                {
                    if (from.IsEmpty())
                    {
                        return true;
                    }

                    if (!from->IsArray())
                    {
                        return false;
                    }

                    v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(from);
                    for (uint32_t i = 0; i < array->Length(); i++)
                    {
                        v8::Local<v8::Value> jsValue = array->Get(i);
                        TElemType nativeValue;
                        JsToNativeConversion<TElemType>()(isolationScope, nativeValue, jsValue);
                        to.push_back(nativeValue);
                    }

                    return true;
                }

                inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
                {
                    return from.IsEmpty() || from->IsArray();
                }
            };

            template<typename TType>
            struct JsToNativeConversion<std::vector<TType> > : public Seq_JsToNativeConversion<std::vector<TType>, TType> { };

            template<typename TType>
            struct JsToNativeConversion<std::list<TType> > : public Seq_JsToNativeConversion<std::list<TType>, TType> { };


#pragma region - std::map
            //-------------------------------------------------
            //  Map
            //-------------------------------------------------

            template<typename TKey, typename TValue>
            struct JsToNativeConversion<std::map<TKey, TValue> >
            {
                inline bool operator()(
                                       Isolate *isolationScope,
                                       std::map<TKey, TValue>& to,
                                       v8::Handle<v8::Value> from)
                {
                    if (from.IsEmpty())
                    {
                        return true;
                    }

                    if (!from->IsObject())
                    {
                        return false;
                    }

                    v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(from);
                    v8::Local<v8::Array> prop_names = obj->GetPropertyNames();

                    for (uint32_t i = 0; i < prop_names->Length(); ++i)
                    {
                        v8::Local<v8::Value> js_name = prop_names->Get(i);
                        v8::Local<v8::Value> js_value = obj->Get(js_name);

                        TKey key;
                        JsToNativeConversion<TKey>()(key, js_name, isolationScope);

                        TValue value;
                        JsToNativeConversion<TValue>()(value, js_value, isolationScope);

                        typedef typename std::map<TKey, TValue>::value_type value_type;
                        to.insert(value_type(key, value));
                    }

                    return true;
                }
            };

            inline static bool isConvertable(Isolate *isolationScope, Handle<Value> from)
            {
                return from.IsEmpty() || from->IsObject();
            }
        }

        //-------------------------------------------------
        //  Declare the common forward API
        //
        //  Note: declaring it here since we'll need it later in this file.
        //-------------------------------------------------

        template <typename TType>
        inline V8_DECL bool JsToNative(Isolate *isolationScope,
                                       TType& to,
                                       v8::Handle<v8::Value> from)
        {
            typedef detail::JsToNativeConversion<TType> TForwarder;
            return TForwarder()(isolationScope, to, from);
        }
        
        template <typename TType>
        inline V8_DECL bool IsJsToNativeConvertable(Isolate *isolationScope,
                                                    v8::Handle<v8::Value> from)
        {
            typedef detail::JsToNativeConversion<TType> TForwarder;
            //v8::String::Utf8Value val(from);
            //std::cout << "val: " << *val << "; type : " << TypeId<TType>().name();
            return TForwarder::isConvertable(isolationScope, from);
        }

    }
}

#endif
