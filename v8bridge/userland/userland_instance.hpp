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


#ifndef BOOST_PP_IS_ITERATING
#   ifndef v8bridge_userland_instance_hpp
#       define v8bridge_userland_instance_hpp

#       include <stdexcept>

#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/conversion.hpp>

#       include <boost/preprocessor/repetition.hpp>
#       include <boost/preprocessor/iteration/iterate.hpp>


namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        
        /**
         * This class represents an instance of a "class" that was declared in the User Land, a.k.a, in JS.
         *
         * This class provides simple interface for interaction with the class instance.
         */
        class V8_DECL UserlandInstance
        {
        public:
            UserlandInstance(Isolate *isolationScope, Handle<Value> objectHandle) :
            m_isolationScope(isolationScope), m_object(isolationScope, Handle<Object>::Cast(objectHandle)) // will result in an error if the handle is not a object
            {
            };
            
            UserlandInstance(Isolate *isolationScope, Handle<Object> objectHandle) :
            m_isolationScope(isolationScope), m_object(isolationScope, objectHandle)
            {
                
            };
            
            ~UserlandInstance()
            {
                 this->m_object.Reset();
            }
            
            inline Local<Object> getObjectHandle() { return Local<Object>::New(this->m_isolationScope, this->m_object); }
            
            //=======================================================================
            //  Instance data getter
            //=======================================================================
            
            inline Local<Value> getHandle(std::string key)
            {
                EscapableHandleScope handle_scope(this->m_isolationScope);
                
                return handle_scope.Escape(
                                           this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, key.c_str()))
                );
            }
            
            template <class TType>
            inline TType getValue(std::string key)
            {
                Local<Value> value =  this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, key.c_str()));
                TType result;
                if (!JsToNative(this->m_isolationScope, result, value))
                {
                    std::stringstream io;
                    io << "The requested variable (" << key << ")"
                    << " does not match the specified TType (" << TypeId<TType>().name() << ").";
                    throw std::runtime_error(io.str());
                }
                return result;
            }
            
            
            //=======================================================================
            //  Instance data setter
            //=======================================================================
            
            inline void setHandle(std::string key, Handle<Value> value)
            {
                EscapableHandleScope handle_scope(this->m_isolationScope);
                
                this->getObjectHandle()->Set(String::NewFromUtf8(this->m_isolationScope, key.c_str()), value);
            }
            
            template <typename TType>
            inline void setValue(std::string key, TType value)
            {
                this->setHandle(key, NativeToJs(this->m_isolationScope, value));
            }
            
            
            //=======================================================================
            //  Invocation
            //=======================================================================
            
            /**
             * Base specification for invoking V8 function (with type conversion).
             */
            // Non-void
            template <class TResult>
            inline typename boost::disable_if<
            boost::is_same<TResult, void>, TResult >::type
            invoke(std::string methodName)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                Handle<Value> method = this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()));
                
#if V8BRIDGE_DEBUG
                if (!method->IsFunction())
                {
                    throw std::runtime_error("The given methodName variable is not a valid method for the given object.");
                }
#else
                assert(method->IsFunction())
#endif
                
                
                Handle<Value> argv[] = {};
                TResult result;
                
                if (!JsToNative(this->m_isolationScope,
                                result, Handle<Function>::Cast(method)->Call(this->getObjectHandle(), 0, argv)))
                {
                    std::stringstream io;
                    io << "The function returned value does not match the specified TResult (" << TypeId<TResult>().name() << ").";
                    throw std::runtime_error(io.str());
                }
                
                return result;
            }
            
            // Void calls
            template <class TResult>
            inline typename boost::enable_if<
            boost::is_same<TResult, void>, TResult >::type
            invoke(std::string methodName)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                Handle<Value> method = this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()));
                Handle<Value> argv[] = {};
                
#if V8BRIDGE_DEBUG
                if (!method->IsFunction())
                {
                    throw std::runtime_error("The given methodName variable is not a valid method for the given object.");
                }
#else
                assert(method->IsFunction())
#endif
                
                Handle<Function>::Cast(method)->Call(this->getObjectHandle(), 0, argv);
            }
            
            /**
             * Raw invocation base specification for invoking V8 function (w/o type conversion).
             */
            inline Handle<Value> rawInvoke(std::string methodName)
            {
                EscapableHandleScope handle_scope(this->m_isolationScope);
                
                Handle<Value> method = this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()));
                Handle<Value> argv[] = {};
                
#if V8BRIDGE_DEBUG
                if (!method->IsFunction())
                {
                    throw std::runtime_error("The given methodName variable is not a valid method for the given object.");
                }
#else
                assert(method->IsFunction())
#endif
                
                return handle_scope.Escape(
                                           Handle<Function>::Cast(method)->Call(this->getObjectHandle(), 0, argv)
                                           );
            }
            
            
#       define BOOST_PP_ITERATION_PARAMS_1 (3, (1, V8_MAX_ARITY, <v8bridge/userland/userland_instance.hpp>))
#       include BOOST_PP_ITERATE()
        private:
            Isolate *m_isolationScope;
            Persistent<Object> m_object;
        };
    }
}

#   endif // v8bridge_userland_instance_hpp
#else // BOOST_PP_IS_ITERATING
#   if BOOST_PP_ITERATION_DEPTH() == 1 // defined(BOOST_PP_IS_ITERATING)
#       define N BOOST_PP_ITERATION()
#       define V8_BRIDGE_CALL_CONCAT_ARG(z, n, data)                    BOOST_PP_CAT(TClass, n) BOOST_PP_CAT(arg, n)
#       define V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE(z, n, data)        NativeToJs<BOOST_PP_CAT(TClass, n)>(this->m_isolationScope, BOOST_PP_CAT(arg, n))

// Non-void
template <class TResult, BOOST_PP_ENUM_PARAMS(N, class TClass) >
inline typename boost::disable_if<
boost::is_same<TResult, void>, TResult >::type
invoke(std::string methodName, BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    HandleScope handle_scope(this->m_isolationScope);
    
    Handle<Value> method = this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()));
    
#if V8BRIDGE_DEBUG
    if (!method->IsFunction())
    {
        throw std::runtime_error("The given methodName variable is not a valid method for the given object.");
    }
#else
    assert(method->IsFunction())
#endif
    
    
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    TResult result;
    
    if (!JsToNative(this->m_isolationScope,
                    result, Handle<Function>::Cast(method)->Call(this->getObjectHandle(), N, argv)))
    {
        std::stringstream io;
        io << "The function returned value does not match the specified TResult (" << TypeId<TResult>().name() << ").";
        throw std::runtime_error(io.str());
    }
    
    return result;
}

// Void calls
template <class TResult, BOOST_PP_ENUM_PARAMS(N, class TClass) >
inline typename boost::enable_if<
boost::is_same<TResult, void>, TResult >::type
invoke(std::string methodName, BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    HandleScope handle_scope(this->m_isolationScope);
    
    Handle<Value> method = this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()));
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    
#if V8BRIDGE_DEBUG
    if (!method->IsFunction())
    {
        throw std::runtime_error("The given methodName variable is not a valid method for the given object.");
    }
#else
    assert(method->IsFunction())
#endif
    
    Handle<Function>::Cast(method)->Call(this->getObjectHandle(), N, argv);
}

// Raw invocation
template <BOOST_PP_ENUM_PARAMS(N, class TClass) >
inline Handle<Value> rawInvoke(std::string methodName, BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    EscapableHandleScope handle_scope(this->m_isolationScope);
    
    Handle<Value> method = this->getObjectHandle()->Get(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()));
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    
#if V8BRIDGE_DEBUG
    if (!method->IsFunction())
    {
        throw std::runtime_error("The given methodName variable is not a valid method for the given object.");
    }
#else
    assert(method->IsFunction())
#endif
    
    return handle_scope.Escape(
                               Handle<Function>::Cast(method)->Call(this->getObjectHandle(), N, argv)
                               );
}

#       undef V8_BRIDGE_CALL_CONCAT_ARG
#       undef V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE
#   endif // BOOST_PP_ITERATION_DEPTH
#endif