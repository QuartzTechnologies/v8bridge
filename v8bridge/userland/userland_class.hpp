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
#   ifndef v8bridge_userland_class_hpp
#       define v8bridge_userland_class_hpp

#       include <stdexcept>

#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/conversion.hpp>

#       include <v8bridge/userland/userland_instance.hpp>

#       include <boost/preprocessor/repetition.hpp>
#       include <boost/preprocessor/iteration/iterate.hpp>
#       include <boost/shared_ptr.hpp>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        
        
        /**
         * This class represents an "class" that was declared in the User Land, a.k.a, in JS.
         *
         * This class provides simple interface for interaction with the declared class.
         */
        class V8_DECL UserlandClass
        {
        public:
            UserlandClass(Isolate *isolationScope, Handle<Value> functionHandle) :
            m_isolationScope(isolationScope), m_function(isolationScope, Handle<Function>::Cast(functionHandle)) // will result in an error if the handle is not a function
            , m_instances(new TInstancesList())
            {
            };
            
            UserlandClass(Isolate *isolationScope, Handle<Function> functionHandle) :
            m_isolationScope(isolationScope), m_function(isolationScope, functionHandle)
            , m_instances(new TInstancesList())
            {
                
            };
            
            ~UserlandClass()
            {
                /* Release the persistent holder */
                this->m_function.Reset();
                
                /* Release each instance */
                
                for (TInstancesList::iterator it = this->m_instances->begin(); it != this->m_instances->end(); ++it)
                {
                    (*it).reset();
                }
                
                this->m_instances->clear();
            }
            
            inline Local<Function> getCtorFunction() { return Local<Function>::New(this->m_isolationScope, this->m_function); }
            
            
            //=======================================================================
            //  General gettter
            //=======================================================================
            
            inline Local<Value> getHandle(std::string key)
            {
                EscapableHandleScope handle_scope(this->m_isolationScope);
                
                return handle_scope.Escape(
                                           this->getCtorFunction()->Get(String::NewFromUtf8(this->m_isolationScope, key.c_str()))
                                           );
            }
            
            template <class TType>
            inline TType getValue(std::string key)
            {
                Local<Value> value =  this->getCtorFunction()->Get(String::NewFromUtf8(this->m_isolationScope, key.c_str()));
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
            //  Instantiation
            //=======================================================================
            
            /**
             * Create a new instance, represented by UserlandInstance class, from the given arguments.
             */
            inline UserlandInstance *newInstance()
            {
                Handle<Value> argv[] = { };
                Handle<Value> v8Instance = this->getCtorFunction()->NewInstance(0, argv);
                
                UserlandInstance *instance = new UserlandInstance(this->m_isolationScope, v8Instance);
                
                boost::shared_ptr<UserlandInstance> adapter(instance);
                
                this->m_instances->push_back(adapter);
                
                return instance;
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
            invoke()
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                Local<Function> callback = this->getCtorFunction();
                Handle<Value> argv[] = {};
                TResult result;
                
                if (!JsToNative(this->m_isolationScope, result, callback->Call(callback, 0, argv)))
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
            invoke()
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                Local<Function> callback = this->getCtorFunction();
                Handle<Value> argv[] = {};
                
                callback->Call(callback, 0, argv);
            }
            
            /**
             * Raw invocation base specification for invoking V8 function (w/o type conversion).
             */
            inline Handle<Value> rawInvoke()
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                Local<Function> callback = this->getCtorFunction();
                Handle<Value> argv[] = {};
                
                return callback->Call(callback, 0, argv);
            }
            
            //=======================================================================
            //  Roll out recursive-iteration to create overloads of newInstance, invoke and rawInvoke.
            //=======================================================================
            
#       define BOOST_PP_ITERATION_PARAMS_1 (3, (1, V8_MAX_ARITY, <v8bridge/userland/userland_class.hpp>))
#       include BOOST_PP_ITERATE()
        private:
            typedef std::list<boost::shared_ptr<UserlandInstance> > TInstancesList;
            
            Isolate *m_isolationScope;
            Persistent<Function> m_function;
            TInstancesList *m_instances;
        };
    }
}

#   endif // v8bridge_userland_class_hpp
#else // BOOST_PP_IS_ITERATING
#   if BOOST_PP_ITERATION_DEPTH() == 1 // defined(BOOST_PP_IS_ITERATING)
#       define N BOOST_PP_ITERATION()
#       define V8_BRIDGE_CALL_CONCAT_ARG(z, n, data)                    BOOST_PP_CAT(TClass, n) BOOST_PP_CAT(arg, n)
#       define V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE(z, n, data)        NativeToJs<BOOST_PP_CAT(TClass, n)>(this->m_isolationScope, BOOST_PP_CAT(arg, n))

//=======================================================================
//  newInstance
//=======================================================================

template <BOOST_PP_ENUM_PARAMS(N, class TClass) >
UserlandInstance *newInstance(BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    Handle<Value> v8Instance = this->getCtorFunction()->NewInstance(N, argv);
    
    UserlandInstance *instance = new UserlandInstance(this->m_isolationScope, v8Instance);
    
    boost::shared_ptr<UserlandInstance> adapter(instance);
    
    this->m_instances->push_back(adapter);
    
    return instance;
}

//=======================================================================
//  invoke
//=======================================================================

// Non-void
template <class TResult, BOOST_PP_ENUM_PARAMS(N, class TClass) >
inline typename boost::disable_if<
boost::is_same<TResult, void>, TResult >::type
invoke(BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    HandleScope handle_scope(this->m_isolationScope);
    
    Local<Function> callback = this->getCtorFunction();
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    
    TResult result;
    
    if (!JsToNative(this->m_isolationScope, result, callback->Call(callback, N, argv)))
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
invoke(BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    HandleScope handle_scope(this->m_isolationScope);
    
    Local<Function> callback = this->getCtorFunction();
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    
    callback->Call(callback, N, argv);
}

//=======================================================================
//  rawInvoke
//=======================================================================

// Raw invocation
template <BOOST_PP_ENUM_PARAMS(N, class TClass) >
inline Handle<Value> rawInvoke(BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONCAT_ARG, ~))
{
    HandleScope handle_scope(this->m_isolationScope);
    
    Local<Function> callback = this->getCtorFunction();
    Handle<Value> argv[] = {
        BOOST_PP_ENUM(N, V8_BRIDGE_CALL_CONVERT_TO_V8_ARG_TYPE, ~)
    };
    
    return callback->Call(callback, N, argv);
}

#   endif // BOOST_PP_ITERATION_DEPTH
#endif
