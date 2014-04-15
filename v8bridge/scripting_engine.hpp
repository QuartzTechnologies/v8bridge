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

#ifndef v8bridge__scripting_engine_hpp
#define v8bridge__scripting_engine_hpp

#include <v8bridge/detail/prefix.hpp>
#include <v8/v8-debug.h>

#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <v8bridge/conversion.hpp>
#include <v8bridge/detail/typeid.hpp>
#include <v8bridge/native/native_class.hpp>
#include <v8bridge/version.hpp>

namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            /* Prototypes */
            void report_v8_exception(v8::Isolate* isolate, v8::TryCatch* try_catch);
            std::string get_report_v8_exception(v8::Isolate* isolate, v8::TryCatch* try_catch);
        }
        
        class V8_DECL ScriptingEngine
        {
        public:
            ScriptingEngine(bool registerBuiltinDeclaration = true)  :
            m_registeredContractsMap(new TNativeContractMap()),
            m_registeredNativeClassesMap(new TNativeClassesContractMap())
            {
                //-------------------------------------------------
                //  Create new isolation scope
                //-------------------------------------------------
                this->m_activeIsolationScope = Isolate::GetCurrent();
                //this->m_activeIsolationScope->Enter();
                
                //-------------------------------------------------
                // Create a temp scope
                //-------------------------------------------------
                HandleScope handle_scope(this->m_activeIsolationScope);
                
                //-------------------------------------------------
                //  Create the active context
                //-------------------------------------------------
                
                // Each processor gets its own context so different engines don't
                // affect each other. Context::New returns a persistent handle which
                // is what we need for the reference to remain after we return from
                // this method. That persistent handle has to be disposed in the
                // destructor.
                Handle<Context> context = Context::New(this->m_activeIsolationScope, NULL);
                context->Enter();
                
                this->m_context.Reset(this->m_activeIsolationScope, context);
                
                //-------------------------------------------------
                // Enter the new context so all the following operations take place
                // within it.
                //-------------------------------------------------
                Context::Scope context_scope(context);
                
                //-------------------------------------------------
                //  Register the engine
                //-------------------------------------------------
                
                s_isolationToEngineMap.insert(std::make_pair(this->m_activeIsolationScope, this));
                
                //-------------------------------------------------
                //  Should we register some built-in functions?
                //-------------------------------------------------
                
                if (registerBuiltinDeclaration)
                {
                    //this->exposeV8Function("sprintf", builtin::sprintf);
                    //this->exposeV8Function("tosource", builtin::js_tosource);
                }
            }
            
            ~ScriptingEngine()
            {
                //-------------------------------------------------
                //  Remove ourselfs from the isolation to engine map
                //-------------------------------------------------
                
                s_isolationToEngineMap.erase(s_isolationToEngineMap.find(this->m_activeIsolationScope));
                
                //-------------------------------------------------
                //  Dispose
                //-------------------------------------------------
                
                // Dispose the persistent handles.  When noone else has any
                // references to the objects stored in the handles they will be
                // automatically reclaimed
                this->m_context.Reset();
                
                //this->m_activeIsolationScope->Exit();
                //this->m_activeIsolationScope->Dispose();
            }
            
            //==========================================================================
            //  Getters & Setters
            //==========================================================================
            inline Isolate *getActiveIsolationScope() { return this->m_activeIsolationScope; }
            //Persistent<Context> getCurrentContext() { return this->*m_context; };
            
            //==========================================================================
            //  Expose
            //==========================================================================
            
            /**
             * Set a given handle on the global scope, associated with the given key (variable name).
             *
             * Example:
             *      C++: setAtGlobalScope("n", v8::Int32::New(isolate, 1));
             *      JS: console.log("passed int value is: " + n);
             */
            inline ScriptingEngine *setAtGlobalScope(std::string key, Handle<Value> value)
            {
                HandleScope handle_scope(this->m_activeIsolationScope);
                
                this->m_activeIsolationScope
                ->GetCurrentContext()
                ->Global()
                ->Set(String::NewFromUtf8(this->m_activeIsolationScope, key.c_str()), value);
                
                return this;
            }
            
            /**
             * Convert the given value to V8 JS handle and set it in the global scope.
             *
             * Example:
             *      C++: setValueAtGlobalScope("n", 1);
             *      JS: console.log("passed int value is: " + n);
             */
            template <typename TType>
            inline ScriptingEngine *setValueAtGlobalScope(std::string key, TType value)
            {
                HandleScope handle_scope(this->m_activeIsolationScope);
                
                this->m_activeIsolationScope
                ->GetCurrentContext()
                ->Global()
                ->Set(String::NewFromUtf8(this->m_activeIsolationScope, key.c_str()), NativeToJs(this->m_activeIsolationScope, value));
                
                return this;
            }
            
            /**
             * Get handle declared on the global scope, associated with the specified key (variable name).
             *
             * Example:
             *      JS: var foo = "Hello, World!";
             *      C++: Handle<String> str = engine->getFromGlobalScope("foo");
             */
            inline Handle<Value> getFromGlobalScope(std::string key)
            {
                //samples/process.cc 192
                EscapableHandleScope handle_scope(this->m_activeIsolationScope);
                
                Local<Value> handle = this->m_activeIsolationScope
                ->GetCurrentContext()
                ->Global()
                ->Get(String::NewFromUtf8(this->m_activeIsolationScope, key.c_str()));
                
                return handle_scope.Escape(handle);
            }
            
            
            /**
             * Get value, converted by the conversion API, declared on the global scope, associated with the specified key (variable name).
             *
             * Example:
             *      JS: var foo = "Hello, World!";
             *      C++: std::string str = engine->getValueFromGlobalScope<std::string>("foo");
             *
             * Note that if the variable does not match the specified TType, an std::runtime_error will be rise.
             */
            template <typename TType>
            inline TType getValueFromGlobalScope(std::string key)
            {
                HandleScope handle_scope(this->m_activeIsolationScope);
                Handle<Value> value = this->getFromGlobalScope(key);
                
                TType resolvedValue;
                if (!JsToNative(this->m_activeIsolationScope, resolvedValue, value))
                {
                    std::stringstream io;
                    io << "The requested variable (" << key << ")"
                        << " does not match the specified TType (" << TypeId<TType>().name() << ").";
                    throw std::runtime_error(io.str());
                }
                
                return resolvedValue;
            }
            
            
            /**
             * Get handle, converted by the conversion API, declared on the global scope, associated with the specified key (variable name).
             *
             * Example:
             *      JS: var foo = [1, 2, 'foo', 'bar'];
             *      C++: Handle<Array> array = engine->getHandleFromGlobalScope<std::string>("foo");
             *
             * Note that if the variable does not match the specified TType, an std::runtime_error will be rise.
             */
            template <typename TType>
            inline TType getHandleFromGlobalScope(std::string key)
            {
                Handle<Value> value = this->getFromGlobalScope(key);
                
                TType resolvedValue;
                if (!JsToNative(this->m_activeIsolationScope, resolvedValue, value))
                {
                    std::stringstream io;
                    io << "The requested variable (" << key << ")"
                    << " does not match the specified TType (" << TypeId<TType>().name() << ").";
                    throw std::runtime_error(io.str());
                }
                
                return resolvedValue;
            }
            
            /**
             * Expose the given function endpoint to JS.
             * For more information on NativeFunction, see native_function.hpp.
             *
             * @param NativeFunction funcDecl - the function declaration. PLEASE NOTE: you should finish to 
             *                       configure the NativeFunction endpoint BEFORE calling this method.
             * @param name - The name of the function, as it will appear in JS.
             */
            inline ScriptingEngine *exposeFunction(NativeFunction *funcDecl, std::string name = "")
            {
                //-------------------------------------------------
                //  Init
                //-------------------------------------------------
                
                HandleScope handle_scope(this->m_activeIsolationScope);
                Handle<Context> context = this->m_activeIsolationScope->GetCurrentContext();
                Context::Scope context_scope(context);
                
#if V8BRIDGE_DEBUG
                assert(this->m_registeredContractsMap->find(name) == this->m_registeredContractsMap->end());
#else
                if (this->m_registeredContractsMap->find(name) != this->m_registeredContractsMap->end())
                {
                    return this;
                }
#endif
                
#if V8BRIDGE_DEBUG
                assert(funcDecl->getOverloadsCount() > 0);
#else
                if (funcDecl->getOverloadsCount() < 1)
                {
                    std::stringstream io;
                    io << "The given function (" << name << ") does not have any registered overload(s)." << std::endl;
                    io << "Before exposing the function, you should register at least one overload using NativeFunction::adOverload.";
                    throw std::runtime_error(io.str());
                }
#endif
                
                //-------------------------------------------------
                //  Create a new adapter using shared pointer
                //-------------------------------------------------
                
                boost::shared_ptr<NativeFunction > adapter(funcDecl);
                
                //-------------------------------------------------
                //  Declare in V8
                //-------------------------------------------------
                
                this->setAtGlobalScope(name, funcDecl->getTemplate()->GetFunction());
                
                //-------------------------------------------------
                //  Register for internal use
                //-------------------------------------------------
                this->m_registeredContractsMap->insert(std::make_pair(name, adapter));
                
                return this;
            }
            
            /**
             * Expose the given class endpoint to JS.
             * For more information on NativeClass, see native_class.hpp.
             *
             * @param NativeClass<TClass> classDecl - the class declaration. PLEASE NOTE: you should finish to
             *          configure the NativeClass endpoint BEFORE calling this method.
             * @param name - The name of the function, as it will appear in JS. If no name was specified, the class name will be used.
             */
            template <class TClass>
            inline ScriptingEngine *exposeClass(NativeClass<TClass> *classDecl, std::string name = "")
            {
                //-------------------------------------------------
                //  Init
                //-------------------------------------------------
                
                HandleScope handle_scope(this->m_activeIsolationScope);
                Handle<Context> context = this->m_activeIsolationScope->GetCurrentContext();
                Context::Scope context_scope(context);
                
                if (name == "")
                {
                    name = TypeId< typename TypeResolver<TClass>::type >().name();
                }
                
                typedef typename TypeResolver<TClass>::type TResolvedType;
                
                //-------------------------------------------------
                //  A class already registered with that name?
                //-------------------------------------------------
                
#if V8BRIDGE_DEBUG
                assert(this->m_registeredNativeClassesMap->find(name) == this->m_registeredNativeClassesMap->end());
#else
                if (this->m_registeredNativeClassesMap->find(name) != this->m_registeredNativeClassesMap->end())
                {
                    return this;
                }
#endif
                
                //-------------------------------------------------
                //  Create a new adapter using shared pointer
                //-------------------------------------------------
                
                boost::shared_ptr<NativeClass<TResolvedType> > adapter(classDecl);
                
                //-------------------------------------------------
                //  Declare in V8
                //-------------------------------------------------
                
                this->setAtGlobalScope(name, classDecl->getTemplate()->GetFunction());
                
                //-------------------------------------------------
                //  Register for internal use
                //-------------------------------------------------
                this->m_registeredContractsMap->insert(std::make_pair(TypeId<TClass>().name(), adapter));
                this->m_registeredNativeClassesMap->insert(std::make_pair(TypeId<TClass>().name(), adapter.get()));
                
                return this;
            }
            
            /**
             * Remove the exposed class from the global scope and erase its tracking data.
             */
            inline ScriptingEngine *removeExposedClass(std::string name = "")
            {
                //-------------------------------------------------
                //  Make sure that this entry exists
                //-------------------------------------------------
#if V8BRIDGE_DEBUG
                assert(this->m_registeredNativeClassesMap->find(name) != this->m_registeredNativeClassesMap->end());
#else
                if (this->m_registeredNativeClassesMap->find(name) == this->m_registeredNativeClassesMap->end())
                {
                    return this;
                }
#endif
                //-------------------------------------------------
                //  Remove from DOM
                //-------------------------------------------------
                
                this->setAtGlobalScope(name, v8::Undefined(this->m_activeIsolationScope));
                
                //-------------------------------------------------
                //  Remove from the lists
                //-------------------------------------------------
                this->m_registeredNativeClassesMap->erase(name);
                this->m_registeredContractsMap->erase(name); // -1 to shared pointer
                
                return this;
            }
            
            //==========================================================================
            // Class contracts (used to convert between types)
            //==========================================================================
            
            template<typename TClass>
            inline NativeEndpoint *getClassContractByType()
            {
                TNativeClassesContractMap::const_iterator it = this->m_registeredNativeClassesMap->find(TypeId<TClass>().name());
                if (it != this->m_registeredNativeClassesMap->end())
                {
                    return it->second;
                }
                
                return 0;
            }
            
            
            //==========================================================================
            //  Execution
            //==========================================================================
            
            inline void execute(const std::string &scriptCode, const std::string &fileName = "")
            {
                HandleScope handle_scope(this->m_activeIsolationScope);
                this->v8Eval(scriptCode, fileName);
            }
            
            template <typename TResult>
            inline TResult eval(const std::string &scriptCode, const std::string &fileName = "")
            {
                HandleScope handle_scope(this->m_activeIsolationScope);
                Handle<Context> context = this->m_activeIsolationScope->GetCurrentContext();
                Context::Scope context_scope(context);
                
                Handle<Value> handle = this->v8Eval(scriptCode, fileName);
                
                TResult result;
                if (!JsToNative<TResult>(this->m_activeIsolationScope, result, handle))
                {
                    String::Utf8Value stringifyResult(handle);
                    std::stringstream io;
                    io << "The evaluated result does not match the specified TResult (" << TypeId<TResult>().name() << ")."
                    << std::endl << "Stingify evaluated result: " << *stringifyResult;
                    throw std::runtime_error(io.str());
                }
                return result;
            }
            
            inline Local<Value> v8Eval(const std::string &scriptCode, const std::string &fileName = "")
            {
                EscapableHandleScope handle_scope(this->m_activeIsolationScope);
                Handle<Context> context = this->m_activeIsolationScope->GetCurrentContext();
                Context::Scope context_scope(context);
                
                TryCatch try_catch;
                //try_catch.SetVerbose(true);
                //try_catch.SetCaptureMessage(true);
                
                Handle<Script> compiledScript;
                if (fileName.empty())
                {
                    compiledScript = Script::Compile(String::NewFromUtf8(this->m_activeIsolationScope, scriptCode.c_str()));
                }
                else
                {
                    compiledScript = Script::Compile(
                                                     String::NewFromUtf8(this->m_activeIsolationScope, scriptCode.c_str()),
                                                     String::NewFromUtf8(this->m_activeIsolationScope, fileName.c_str())
                                                     );
                }
                
                if (compiledScript.IsEmpty()) {
                    String::Utf8Value error(try_catch.Exception());
#if V8BRIDGE_DEBUG
                    detail::report_v8_exception(this->m_activeIsolationScope, &try_catch);
#endif
                    return v8::Undefined(this->m_activeIsolationScope);
                }
                
                Local<Value> result = compiledScript->Run();
                
                if (result.IsEmpty())
                {
                    assert(try_catch.HasCaught());
#if V8BRIDGE_DEBUG
                    detail::report_v8_exception(this->m_activeIsolationScope, &try_catch);
#endif
                    return v8::Undefined(this->m_activeIsolationScope);
                }
                
                /* Since we're receiving a Local object, we must escape it before retuning,
                 otherwise the current HandleScope will dispose it. */
                return handle_scope.Escape(result);
            }
            
            //==========================================================================
            //  Misc
            //==========================================================================
            inline std::string getStackTrace(int limit = 10)
            {
                HandleScope handle_scope(this->m_activeIsolationScope);
                
                Handle<StackTrace> stackTrace = StackTrace::CurrentStackTrace(this->m_activeIsolationScope, limit, StackTrace::kOverview);
                
                std::ostringstream io;
                for (int i = 0; i < stackTrace->GetFrameCount(); ++i)
                {
                    Handle<StackFrame> frame = stackTrace->GetFrame(i);
                    
                    io << *String::Utf8Value(frame->GetFunctionName())
                    << " [line: " << frame->GetLineNumber() << ", column: " << frame->GetColumn() << "]"
                    << std::endl;
                }
                return io.str();
            }
            
            //==========================================================================
            //  Static helpers
            //==========================================================================
            
            /**
             * Get the scripting engine instance associated with the given isolation scope
             */
            inline static ScriptingEngine *EngineFromIsolationScope(Isolate *isolate)
            {
                std::map<Isolate *, ScriptingEngine *>::const_iterator it = s_isolationToEngineMap.find(isolate);
                if (it == s_isolationToEngineMap.end())
                {
                    return 0;
                }
                
                return it->second;
            }
            
            /**
             * Get the library current version
             */
            inline static std::string GetVersion()
            {
                return V8BRIDGE_VERSION_STRING;
            }
            
        private:
            /* Types */
            typedef std::map<std::string, boost::shared_ptr<NativeEndpoint> > TNativeContractMap;
            typedef std::map<std::string, NativeEndpoint *> TNativeClassesContractMap;
            
            /* Static members */
            static std::map<Isolate *, ScriptingEngine *> s_isolationToEngineMap;
            
            /* Private members */
            Persistent<Context> m_context;
            Isolate *m_activeIsolationScope;
            
            TNativeContractMap *m_registeredContractsMap;
            TNativeClassesContractMap *m_registeredNativeClassesMap;
        };
        
        std::map<Isolate *, ScriptingEngine *> ScriptingEngine::s_isolationToEngineMap;
        
        //==========================================================================
        //  Private helpers
        //==========================================================================
        namespace detail
        {
            /**
             * Report the user about the thrown exception.
             * @param Isolate isolate - The used isolation scope.
             * @param TryCatch try_catch - The used try-catch block
             *
             * This implementation taken from V8 samples - shell.cc.
             * See: http://v8.googlecode.com/svn/trunk/samples/shell.cc
             */
            void report_v8_exception(v8::Isolate* isolate, v8::TryCatch* try_catch) {
                v8::HandleScope handle_scope(isolate);
                v8::String::Utf8Value exception(try_catch->Exception());
                const char* exception_string = *exception ? *exception : "<string conversion failed>";
                v8::Handle<v8::Message> message = try_catch->Message();
                if (message.IsEmpty()) {
                    // V8 didn't provide any extra information about this error; just
                    // print the exception.
                    fprintf(stderr, "%s\n", exception_string);
                } else {
                    // Print (filename):(line number): (message).
                    v8::String::Utf8Value filename(message->GetScriptResourceName());
                    const char* filename_string = *filename ? *filename : "<string conversion failed>";
                    int linenum = message->GetLineNumber();
                    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
                    // Print line of source code.
                    v8::String::Utf8Value sourceline(message->GetSourceLine());
                    const char* sourceline_string = *sourceline ? *sourceline : "<string conversion failed>";
                    fprintf(stderr, "%s\n", sourceline_string);
                    // Print wavy underline (GetUnderline is deprecated).
                    int start = message->GetStartColumn();
                    for (int i = 0; i < start; i++) {
                        fprintf(stderr, " ");
                    }
                    int end = message->GetEndColumn();
                    for (int i = start; i < end; i++) {
                        fprintf(stderr, "^");
                    }
                    fprintf(stderr, "\n");
                    v8::String::Utf8Value stack_trace(try_catch->StackTrace());
                    if (stack_trace.length() > 0) {
                        const char* stack_trace_string = *stack_trace ? *stack_trace : "<string conversion failed>";
                        fprintf(stderr, "%s\n", stack_trace_string);
                    }
                }
            }
            
            /* Hand-rolled version of ReportException that allows to receive the value as string
             instead of echoing it to stderr. */
            inline static std::string get_reported_v8_exception(v8::Isolate* isolate, v8::TryCatch* try_catch) {
                std::stringstream io;
                
                v8::HandleScope handle_scope(isolate);
                v8::String::Utf8Value exception(try_catch->Exception());
                const char* exception_string = *exception ? *exception : "<string conversion failed>";
                v8::Handle<v8::Message> message = try_catch->Message();
                if (message.IsEmpty()) {
                    // V8 didn't provide any extra information about this error; just
                    // print the exception.
                    io << exception_string << std::endl;
                } else {
                    // Print (filename):(line number): (message).
                    v8::String::Utf8Value filename(message->GetScriptResourceName());
                    const char* filename_string = *filename ? *filename : "<string conversion failed>";
                    int linenum = message->GetLineNumber();
                    io << filename_string << ":" << linenum << ": " << exception_string << std::endl;
                    // Print line of source code.
                    v8::String::Utf8Value sourceline(message->GetSourceLine());
                    const char* sourceline_string = NativeToString(isolate, *sourceline).c_str();
                    io << sourceline_string << std::endl;
                    // Print wavy underline (GetUnderline is deprecated).
                    int start = message->GetStartColumn();
                    for (int i = 0; i < start; i++) {
                        io << " ";
                    }
                    int end = message->GetEndColumn();
                    for (int i = start; i < end; i++) {
                        io << "^";
                    }
                    io << std::endl;
                    v8::String::Utf8Value stack_trace(try_catch->StackTrace());
                    if (stack_trace.length() > 0) {
                        const char* stack_trace_string = *stack_trace ? *stack_trace : "<string conversion failed>";
                        io << stack_trace_string << std::endl;
                    }
                }
                
                return io.str();
            }
            
            
            //-------------------------------------------------
            //  Allow to convert between custom types
            //
            //  Yes. This piece belongs to NativeToJsConversion.
            //  I've put it here since we must have access to %ScriptingEngine.
            //  We need this class to access the registered classes contracts map.
            //-------------------------------------------------
#pragma region - Custom type casting
            
            template<typename TType>
            struct Pointer_NativeToJsConversion
            {
                inline v8::Handle<v8::Value> operator()(
                                                        v8::Isolate *isolationScope,
                                                        TType *value)
                {
                    typedef typename TypeResolver<TType>::type TResolvedType;
                    
                    ScriptingEngine *engine = ScriptingEngine::EngineFromIsolationScope(isolationScope);
                    
                    NativeClass<TType> *adapter = static_cast<NativeClass<TType> *>(
                                                                                    engine->getClassContractByType<TResolvedType>()
                                                                                    );
                    
                    return adapter->wrap(value);
                    
                    //Handle<ObjectTemplate> instanceTemplate = adapter->getTemplate()->InstanceTemplate();
                    //Local<Object> object = instanceTemplate->NewInstance();
                    
                    /* http://stackoverflow.com/questions/21239249/storing-handles-to-objects-in-a-hashmap-or-set-in-googles-v8-engine */
                    //v8::Persistent<Object, CopyablePersistentTraits<Function> > persistentObject(isolationScope, object);
                    
                    //persistentObject.SetWeak(value, &NativeClass<TType>::WeakObjectsDeletionCallback);
                    
                    //return object;
                }
            };
            
            /* Apply */
            template<typename TType>
            struct NativeToJsConversion<TType *> : public Pointer_NativeToJsConversion<TType> { };
            template<typename TType>
            struct NativeToJsConversion<const TType *> : public Pointer_NativeToJsConversion<TType> { };
        }
    }
}

#endif /* defined(__V8Bridge__ScriptingEngine__) */
