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
 * This file contains some useful macros used to ease the interaction with V8 API.
 *
 * Please note that there's no use for this file within v8bridge.
 */

#ifndef v8bridge_utilities_hpp
#   define v8bridge_utilities_hpp

//=======================================================================
//  Missing macros
//=======================================================================
#   ifndef MIN
#       define MIN(X,Y)                     ((X) < (Y) ? (X) : (Y))
#   endif

#   ifndef MAX
#       define MAX(X,Y)                     ((X) > (Y) ? (X) : (Y))
#   endif

//=======================================================================
//  Basic interaction with V8
//=======================================================================

/* Default */
#   define JS_INT(n)                        ::v8::Int32::New(::v8::Isolate::GetCurrent(), n)
#   define JS_STR(value)                    ::v8::String::New(::v8::Isolate::GetCurrent(), value)
#   define JS_DOUBLE(n)                     ::v8::Number::New(::v8::Isolate::GetCurrent(), n)
#   define JS_BOOL(value)                   ::v8::Boolean::New(::v8::Isolate::GetCurrent(), value)

#   define JS_TRUE                          ::v8::True(::v8::Isolate::GetCurrent())
#   define JS_FALSE                         ::v8::False(::v8::Isolate::GetCurrent())

#   define JS_NULL                          ::v8::Null(::v8::Isolate::GetCurrent())
#   define JS_UNDEFINED                     ::v8::Undefined(::v8::Isolate::GetCurrent())

/* Macros with isolate parameter */
#   define JS_STR_AUX(isolate, value)       ::v8::String::New(isolate, value)
#   define JS_INT_AUX(isolate, n)           ::v8::Int32::New(isolate, n)
#   define JS_DOUBLE_AUX(isolate, n)        ::v8::Number::New(isolate, n)
#   define JS_BOOL_AUX(isolate, value)      ::v8::Boolean::New(isolate, value)

#   define JS_TRUE_AUX(isolate)             ::v8::Undefined(isolate)
#   define JS_FALSE_AUX(isolate)            ::v8::False(isolate)

#   define JS_NULL_AUX(isolate)             ::v8::Null(isolate)
#   define JS_UNDEFINED_AUX(isolate)        ::v8::Undefined(isolate)

//=======================================================================
//  V8 objects
//=======================================================================

/* Get the stored underlaying C++ binded class from Handle<Object> */

// Pointer of type "void *"
#   define NATIVE_BINDED_OBJECT_PTR_FROM_HANDLE(handle) \
                                            handle->GetAlignedPointerFromInternalField( handle->InternalFieldCount() - 2 )

// Pointer of type "type"
#   define NATIVE_BINDED_OBJECT_FROM_HANDLE(handle, type) \
                                            static_cast<type *>(NATIVE_BINDED_OBJECT_PTR_FROM_HANDLE(handle))

/* Get the stored owner NativeClass<TClass> for the given Handle<Object> */
#   define BRIDGE_NATIVE_CLASS_FROM_HANDLE(handle, type) \
                                            static_cast<NativeClass<type> *>(handle->GetAlignedPointerFromInternalField( handle->InternalFieldCount() - 1 ))

/* Dispose the given Handle<Object> and free the underlaying binded C++ object */
#   define DISPOSE_OBJECT_HANDLE(handle, type) \
                                            BRIDGE_NATIVE_CLASS_FROM_HANDLE(handle, type)->disposeInstance(handle)

//=======================================================================
//  Misc
//=======================================================================

#   define JS_OBJ_INSTANCE_OF(obj, func)    func->HasInstance(obj)
#   define JS_METHOD(name)                  void name(const ::v8::FunctionCallbackInfo<Value> &info)

#   define NATIVE_CLASS_FROM_HANDLE(handle, type) \
                                            static_cast<type *>

//=======================================================================
//  Quick exposure
//=======================================================================

/* Expose the specificied function in the specified engine.
    The function will be available in JS at the same name as it was declared in C++. */
#   define EXPOSE_NATIVE_FUNCTION(engine, cppFunctionPointer) \
                                            do \
                                            { \
                                                NativeFunction *__func = new NativeFunction(engine->getActiveIsolationScope()); \
                                                __func->addOverload(cppFunctionPointer); \
                                                engine->exposeFunction(""cppFunctionPointer"", cppFunctionPointer); \
                                            } while (0);

#endif
