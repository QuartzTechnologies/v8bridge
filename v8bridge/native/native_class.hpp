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

#ifndef v8bridge_native_class_hpp
#define v8bridge_native_class_hpp

#include <boost/shared_ptr.hpp>
#include <v8bridge/detail/prefix.hpp>

#include <v8bridge/detail/typeid.hpp>
#include <v8bridge/conversion/type_resolver.hpp>

#include <v8bridge/detail/signature.hpp>
#include <v8bridge/detail/ctor.hpp>
#include <v8bridge/native/invoke_native_ctor.hpp>
#include <v8bridge/native/native_endpoint.hpp>
#include <v8bridge/native/native_function.hpp>
#include <v8bridge/native/native_ctor.hpp>
#include <v8bridge/detail/internal_gc.hpp>
#include <v8bridge/conversion.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace v8
{
    namespace bridge
    {
        /**
         * Specific class used to provide interface for binding C++ classes to JS function (classes).
         * This class also automaticly connects the C++ type with JsToNative and NativeToJs conversion API
         * so the class instances can be passed between C++ and JS.
         *
         * Simple usage:
         *  C++:
         *      NativeClass<Car> *carClass = new NativeClass<Car>();
         *          carClass
         *                  ->setConst("WHEELS", 4)
         *                  ->exposeMethod("drive", &Car::drive)
         *                  ->exposeMethod("stop", &Car::stop)
         *                  ->exposeStaticMethod("Factory", &Car::Factory);
         *                  ->exposePropertyAccessor("color", &Car::getColor, &Car::setColor);
         *
         *  JS:
         *          var car = new Car();
         *          car.color = 'Red';
         *          car.drive();
         *          car.stop();
         *          console.log("This is %s car which has %d wheels.", car.color, Car.WHEELS);
         */
        template <class TClass>
        class V8_DECL NativeClass : public NativeEndpoint
        {
        public:
            NativeClass(Isolate *isolationScope) : NativeEndpoint(isolationScope),
            m_ctor(new NativeCtor(isolationScope)),
            m_methods(new TMethodsMap()),
            m_staticMethods(new TMethodsMap()),
            m_accessors(new TMethodsMap()),
            m_staticAccessors(new TMethodsMap()),
            m_gc(new GC(isolationScope)),
            m_customDtorHandler(NULL)
            {
                HandleScope handle_scope(isolationScope);
                
                this->m_isAbstract = false;
                Local<FunctionTemplate> templ = FunctionTemplate::New(
                                                                      /* isolate: */this->m_isolationScope,
                                                                      /* callback (used as constructor) */ &NativeClass<TClass>::internalConstructorInvocationCallback,
                                                                      /* data: */External::New(isolationScope, (void *)this));
                
                this->m_templateDecl = new Eternal<FunctionTemplate>(this->m_isolationScope, templ);
                this->setInternalFieldsCount(0);
                this->setClientClassName(TypeId< typename TypeResolver<TClass>::type >().name());
            }
            
            ~NativeClass()
            {
                /* Registered ctor */
                delete this->m_ctor;
                
                /* Registered methods */
                for (TMethodsMap::iterator it = this->m_methods->begin(); it != this->m_methods->end(); ++it)
                {
                    (*it).second.reset();
                }
                
                this->m_methods->clear();
                delete this->m_methods;
                
                
                /* Registered static methods */
                for (TMethodsMap::iterator it = this->m_staticMethods->begin(); it != this->m_staticMethods->end(); ++it)
                {
                    (*it).second.reset();
                }
                
                this->m_staticMethods->clear();
                delete this->m_staticMethods;
                
                /* Registered accessors */
                for (TMethodsMap::iterator it = this->m_accessors->begin(); it != this->m_accessors->end(); ++it)
                {
                    (*it).second.reset();
                }
                
                this->m_accessors->clear();
                delete this->m_accessors;
                
                
                /* Registered static accessors */
                for (TMethodsMap::iterator it = this->m_staticAccessors->begin(); it != this->m_staticAccessors->end(); ++it)
                {
                    (*it).second.reset();
                }
                
                this->m_staticAccessors->clear();
                delete this->m_staticAccessors;
                
                /* GC */
                delete this->m_gc;
                
                /* Ctor */
                delete this->m_templateDecl;
            }
            
            //=======================================================================
            // General
            //=======================================================================
            
            /**
             * Get the class JS function template
             */
            inline Handle<FunctionTemplate> getTemplate() { return this->m_templateDecl->Get(this->m_isolationScope); }
            
            /**
             * Test if a given handle is an instance of this class.
             */
            inline bool isInstanceOf(Handle<Value> value)
            {
                return this->getTemplate()->HasInstance(value);
            }
            
            /**
             * Set the function client-side class name (e.g. "Foo" will result in [object Foo])
             */
            inline NativeClass<TClass> *setClientClassName(std::string name)
            {
                this->getTemplate()->SetClassName(String::NewFromUtf8(this->m_isolationScope, name.c_str()));
                return this;
            }
            
            /**
             * Declare this JS class as non-instanciatable.
             */
            inline NativeClass<TClass> declareAsAbstract(bool status = true)
            {
                this->m_isAbstract = status;
                return this;
            }
            
            
            /**
             * Check if the native class was declared as abstract class
             */
            inline bool isDeclaredAsAbstract()
            {
                return this->m_isAbstract;
            }
            
            /**
             * Declares a custom destructor implementation.
             * The given dtor will be called when the object should be free'd (for instance, by the GC).
             *
             * Note that if you will not specify any custom dtor, the default implementation (GCGenericInstanceDestructor declared in internal_gc.hpp)
             * will attempt to dispose the instance using "delete" (and as a result will call the class destructor.
             */
            inline NativeClass<TClass> *declareCustomDtor(GC::TDtor dtor)
            {
                this->m_customDtorHandler = dtor;
            }
            
            //=======================================================================
            //  Exposing Ctor(s)
            //=======================================================================
            
            template <class TCtor>
            inline NativeClass<TClass> *exposeCtor(TCtor const& constructor)
            {
                constructor.define(*this);
                return this;
            }
            
            // Builds ctor function which inserts the given Holder type
            // in a wrapped C++ class instance. ArgList is an MPL type sequence
            // describing the C++ argument types to be passed to Holder's
            // constructor.
            //
            // Holder and ArgList are intended to be explicitly specified.
            template <class TArgList, class TArity>
            inline NativeClass<TClass> *exposeCtor(TArgList* = 0, TArity* = 0)
            {
                return this->_exposeCtor(
                                         ::v8::bridge::detail::invoke_native_ctor<TArity::value>::template apply<TClass, TArgList>::execute,
                                         get_signature(::v8::bridge::detail::invoke_native_ctor<TArity::value>::template apply<TClass, TArgList>::execute)
                                         );
            }
            
            //=======================================================================
            //  Exposing Class Member(s)
            //=======================================================================
            
            /**
             * Add a class member (ivar) to the class.
             *
             * Example:
             *  C++:
             *      obj->setClassMember("x", 10);
             *
             *  JS:
             *      log(obj.x)
             *      obj.x = 5;
             */
            template<typename TType>
            inline NativeClass<TClass> *setClassMember(const char *memberName, TType value)
            {
                this->setClassMember(memberName, NativeToJs(value, this->m_isolationScope));
                return this;
            }
            
            
            template<typename TType>
            inline NativeClass<TClass> *setStaticClassMember(const char *memberName, TType value)
            {
                this->setStaticClassMember(memberName, NativeToJs(value, this->m_isolationScope));
                return this;
            }
            
            
            inline NativeClass<TClass> *setClassMember(const char *memberName, Handle<Value> value)
            {
                this->getTemplate()
                ->InstanceTemplate()
                ->Set(String::NewFromUtf8(this->m_isolationScope, memberName), value);
                
                return this;
            }
            
            inline NativeClass<TClass> *setStaticClassMember(const char *memberName, Handle<Value> value)
            {
                this->getTemplate()
                ->Set(String::NewFromUtf8(this->m_isolationScope, memberName), value);
                
                return this;
            }
            
            
            //=======================================================================
            //  Exposing Method(s)
            //=======================================================================
            
            /**
             * Expose a static constant to JavaScript.
             *
             * Example:
             *  C++:
             *      obj->exposeStaticConstant("WHEELS", 4);
             *
             *  JS:
             *      log(obj.WHEELS);
             */
            inline NativeClass<TClass> *exposeStaticConstant(std::string name, int value)
            {
                this->getTemplate()->GetFunction()->Set(String::NewFromUtf8(this->m_isolationScope, name.c_str()),
                                                        Number::New(this->m_isolationScope, value),
                                                        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
                return this;
            }
            
            /**
             * Expose a specific method or method overload to JavaScript.
             * Note: sending the same methodName will allow to register an overload.
             *
             * @param std::string methodName - The JS method name.
             * @param TMethod method - reference to the C++ method.
             * @return NativeClass<TClass>
             *
             * Example:
             *  C++:
             *      carClass->exposeMethod("drive", &Car::drive);
             *      carClass->exposeStaticMethod("foo", &Car::foo);
             *  JS:
             *      var myCar = new Car();
             *      myCar.drive();
             *      Car.foo();
             */
            template<typename TMethod>
            inline NativeClass<TClass> *exposeMethod(std::string methodName, TMethod method)
            {
                return this->_exposeMethod(methodName, method, /* isStatic: */false);
            }
            
            template<typename TMethod>
            inline NativeClass<TClass> *exposeStaticMethod(std::string methodName, TMethod method)
            {
                return this->_exposeMethod(methodName, method, /* isStatic: */true);
            }
            
            inline NativeClass<TClass> *exposeMethod(std::string methodName, NativeFunction *funcDecl)
            {
                assert(this->m_methods->find(methodName) == this->m_methods->end());
                
                /* Create a shared pointer */
                boost::shared_ptr<NativeFunction> adapter(funcDecl);
                
                /* Store */
                this->m_methods->insert(std::make_pair(methodName, adapter));
                
                /* Add to the instance template */
                this->getTemplate()
                    ->InstanceTemplate()
                    ->Set(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()), funcDecl->getTemplate()->GetFunction());
                
                return this;
            }
            
            inline NativeClass<TClass> *exposeStaticMethod(std::string methodName, NativeFunction *funcDecl)
            {
                assert(this->m_staticMethods->find(methodName) == this->m_staticMethods->end());
                
                /* Create a shared pointer */
                boost::shared_ptr<NativeFunction> adapter(funcDecl);
                
                /* Store */
                this->m_staticMethods->insert(std::make_pair(methodName, adapter));
                
                /* Add to the instance template */
                this->getTemplate()
                    ->Set(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()), funcDecl->getTemplate()->GetFunction());
                
                return this;
            }
            
            /**
             * Retrieve the registered NativeFunction instance for the given method name
             * @param std::string methodName - the method name.
             * @return NativeFunction - the native function data to expose.
             */
            inline NativeFunction *getExposedMethod(std::string methodName)
            {
                if (this->m_methods->find(methodName) == this->m_methods->end())
                {
                    return NULL;
                }
                
                return this->m_methods->find(methodName).second;
            }
            
            inline NativeFunction *getExposedStaticMethod(std::string methodName)
            {
                if (this->m_staticMethods->find(methodName) == this->m_staticMethods->end())
                {
                    return NULL;
                }
                
                return this->m_staticMethods->find(methodName).second;
            }
            
            
            //=======================================================================
            //  Exposing Accessor(s)
            //=======================================================================
            
            /**
             * Expose a specific property to JavaScript.
             *
             * For details, please see: NativeClass<TClass>::exposePropertyAccessor<TGetter, TSetter>(std::string, TGetter, TSetter, std::string, std::string)
             *
             * Note: This version expose a "read-only" property.
             * For read-and-write you should use exposePropertyAccessor<TGetter, TSetter>(std::string, TGetter, TSetter)
             *
             * @param std::string propertyName - the name of the property (variable) in JS.
             * @param TGetter getter - a reference to the getter method.
             * @param std::string getterMethodName [optional] - If specified, in addition to the property accessor declaration,
             *        we're also binding the referenced TGetter to the given method name (i.e. one can access using the C++ method by "x" and "getX").
             */
            template<typename TGetter>
            inline NativeClass<TClass> *exposePropertyAccessor(std::string propertyName, TGetter getter, std::string getterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getter, getterMethodName, /* isStatic: */false);
            }
            
            template<typename TGetter>
            inline NativeClass<TClass> *exposeStaticPropertyAccessor(std::string propertyName, TGetter getter, std::string getterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getter, getterMethodName, /* isStatic: */true);
            }
            
            inline NativeClass<TClass> *exposePropertyAccessor(std::string propertyName, NativeFunction *getterMethod, std::string getterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getterMethod, getterMethodName, /* isStatic: */false);
            }
            
            inline NativeClass<TClass> *exposeStaticPropertyAccessor(std::string propertyName, NativeFunction *getterMethod, std::string getterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getterMethod, getterMethodName, /* isStatic: */true);
            }
            
            /**
             * Expose a specific property to JavaScript.
             *
             * C++ doesn't have "properties" (unlike C#) but we still use getters and setters.
             * Using this API you can attach specific getter and setter to a "property" (i.e. variable).
             *
             * @param std::string propertyName - the name of the property (variable) in JS.
             * @param TGetter getter - a reference to the getter method.
             * @param TSetter setter - a reference to the setter method.
             * @param std::string getterMethodName [optional] - If specified, in addition to the property accessor declaration,
             *        we're also binding the referenced TGetter to the given method name (i.e. one can access using the C++ method by "x" and "getX").
             * @param std::string setterMethodName [optional] - If specified, in addition to the property accessor declaration,
             *        we're also binding the referenced TSetter to the given method name.
             *
             * Example:
             *  C++:
             *      carClass->exposePropertyAccessor("color", &Car::getColor, &Car::setColor);
             *  JS:
             *      var car = new Car();
             *      car.color = "Red"; // Invokes C++'s Car::setColor
             *      console.log( car.color ); // Invokes C++'s Car::getColor
             */
            template<typename TGetter, typename TSetter>
            inline NativeClass<TClass> *exposePropertyAccessor(std::string propertyName, TGetter getter, TSetter setter, std::string getterMethodName = "", std::string setterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getter, setter, getterMethodName, setterMethodName, /* isStatic: */false);
            }
            
            template<typename TGetter, typename TSetter>
            inline NativeClass<TClass> *exposeStaticPropertyAccessor(std::string propertyName, TGetter getter, TSetter setter, std::string getterMethodName = "", std::string setterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getter, setter, getterMethodName, setterMethodName, /* isStatic: */true);
            }
            
            inline NativeClass<TClass> *exposePropertyAccessor(std::string propertyName, NativeFunction *getterMethod, NativeFunction *setterMethod, std::string getterMethodName = "", std::string setterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getterMethod, setterMethod, getterMethodName, setterMethodName, /* isStatic: */false);
            }
            
            inline NativeClass<TClass> *exposeStaticPropertyAccessor(std::string propertyName, NativeFunction *getterMethod, NativeFunction *setterMethod, std::string getterMethodName = "", std::string setterMethodName = "")
            {
                return this->_exposePropertyAccessor(propertyName, getterMethod, setterMethod, getterMethodName, setterMethodName, /* isStatic: */true);
            }
            
            inline NativeFunction *getExposedGetterAccessor(std::string propertyName)
            {
                propertyName = "__get_accessor_" + propertyName;
                
                if (this->m_accessors->find(propertyName) == this->m_accessors->end())
                {
                    return NULL;
                }
                
                return this->m_accessors->find(propertyName).second;
            }
            
            
            inline NativeFunction *getExposedStaticGetterAccessor(std::string propertyName)
            {
                propertyName = "__get_accessor_" + propertyName;
                
                if (this->m_staticAccessors->find(propertyName) == this->m_accessors->end())
                {
                    return NULL;
                }
                
                return this->m_staticAccessors->find(propertyName).second;
            }
            
            
            inline NativeFunction *getExposedSetterAccessor(std::string propertyName)
            {
                propertyName = "__set_accessor_" + propertyName;
                
                if (this->m_accessors->find(propertyName) == this->m_accessors->end())
                {
                    return NULL;
                }
                
                return this->m_accessors->find(propertyName).second;
            }
            
            
            inline NativeFunction *getExposedStaticSetterAccessor(std::string propertyName)
            {
                propertyName = "__set_accessor_" + propertyName;
                
                if (this->m_staticAccessors->find(propertyName) == this->m_accessors->end())
                {
                    return NULL;
                }
                
                return this->m_staticAccessors->find(propertyName).second;
            }
            
            //=======================================================================
            //  Wrapping & Unwrapping objects
            //=======================================================================
            inline Local<Object> wrap(TClass *connectedInstance)
            {
                EscapableHandleScope handle_scope(this->m_isolationScope);
                
                Handle<Value> argv[] = { External::New(this->m_isolationScope, (void*)connectedInstance) };
                
                Local<Object> obj = this->getTemplate()
                    ->GetFunction()
                    ->NewInstance(1, argv);
                
                return handle_scope.Escape(obj);
            }
            
            inline TClass *unwrap(Handle<Object> object)
            {
                return static_cast<TClass *>(object->GetAlignedPointerFromInternalField(object->InternalFieldCount() - 2));
            }
            
            //=======================================================================
            //  V8 API related
            //=======================================================================
            
            /**
             * Use this method to setup the allocated memory adjustment that should be make in
             * V8 when creating new instance.
             *
             * For more details about the memory adjustment, see Isolate::AdjustAmountOfExternalAllocatedMemory
             * 
             * You should call this method only when setting-up the NativeClass with fixed value. Otherwise, it may lead to memory-leak mistakes.
             *
             * Default adjustment:
             *  If you wouldn't call this method at all, the default implementation use sizeof(TClass), which should be just fine for most cases.
             * 
             * Zero:
             *  Passing zero will cause to no memory ajustment been made at all.
             */
            inline NativeClass<TClass> *setAllocatedMemoryAdjustment(size_t cost)
            {
                this->m_allocatedMemoryAdjustment = cost;
            }
            
            /**
             * Set the number of internal fields the native class should reserve.
             * For basic knowladge on internal fields,
                please see V8's SetInternalFieldCount, SetAlignedPointerAtInternalField, GetIntenralPointerFromInternalField, SetInternalField and GetIntenralField.
             *
             * You should note that the internal fields count is the number of fields that you wish to reserve in order to been used BY YOU, not by v8bridge.
             * v8bridge reserves 2 internal fields, so in case you're not reserving anything (0) the number of internal fields will be 2,
             *  in case you reserve 2 fields the internal fields count will be 4 and so on.
             *
             * v8bridge internal fields description:
             *      1'th field (N + 0'th index): A pointer to the binded (connected) C++ class instance.
             *      2'th field (N + 1'th index): A pointer to the owner NativeClass instance.
             */
            inline NativeClass<TClass> *setInternalFieldsCount(int fieldsCount)
            {
                this->m_templateDecl->Get(this->m_isolationScope)->InstanceTemplate()->SetInternalFieldCount(fieldsCount + 2);
                this->m_templateDecl->Get(this->m_isolationScope)->PrototypeTemplate()->SetInternalFieldCount(fieldsCount + 2);
                
                return this;
            }
            
            //=======================================================================
            //  Helpers
            //=======================================================================
            
            /**
             * Dispose the given object handle instance and free the underlaying C++ binded instance.
             */
            inline NativeClass<TClass> *disposeInstance(Handle<Object> handle)
            {
                v8::HandleScope handle_scope(this->m_isolationScope);
                Persistent<Object> p(this->m_isolationScope, handle);
                
                void* ptr = handle->GetAlignedPointerFromInternalField(handle->InternalFieldCount() - 2);
                
                this->m_gc->disposeAndDequeue(ptr);
                
                for (int i = 0; i < handle->InternalFieldCount(); i++)
                {
                    handle->SetAlignedPointerInInternalField(i, NULL);
                }
                p.Reset();
                
                /* Memory adjustment */
                this->m_isolationScope->AdjustAmountOfExternalAllocatedMemory(this->m_allocatedMemoryAdjustment * -1);
                
                return this;
            }
            
            /**
             * Dispose the given object handle instance and free the underlaying C++ binded instance.
             */
            inline NativeClass<TClass> *disposeInstance(Handle<Value> handle)
            {
                if (handle.IsEmpty() || ! handle->IsObject()) { return this; }
                return this->disposeInstance(Handle<Object>(Object::Cast(*handle)));
            }
            
            
            /**
             * Static callback used with Persistent<T>.SetWeak in order to
             * delete references.
             */
            template <typename T, typename P>
            inline static void WeakObjectsDeletionCallback(const WeakCallbackData<T, P>& data)
            {
                Local<Object> pobj = data.GetValue();
                NativeClass<TClass> *self = static_cast<NativeClass<TClass> *>(pobj->GetAlignedPointerFromInternalField(pobj->InternalFieldCount() - 1));
               
                self->disposeInstance(pobj);
            }
            
            
        private:
            typedef std::list<boost::shared_ptr<NativeFunction> > TMethodsList;
            typedef std::map<std::string, boost::shared_ptr<NativeFunction> > TMethodsMap;
            
            NativeCtor *m_ctor;
            
            TMethodsMap *m_methods;
            TMethodsMap *m_staticMethods;
            TMethodsMap *m_accessors;
            TMethodsMap *m_staticAccessors;
            
            bool m_isAbstract;
            int m_internalFieldsCount = 0;
            
            mutable Eternal<FunctionTemplate> *m_templateDecl;
            
            size_t m_allocatedMemoryAdjustment = sizeof(TClass);
            GC *m_gc;
            GC::TDtor m_customDtorHandler;
            
            inline static void internalConstructorInvocationCallback(const FunctionCallbackInfo<Value>& info)
            {
                using namespace boost;
                
                NativeClass<TClass> *self = static_cast<NativeClass<TClass> *>(External::Cast(*info.Data())->Value());
                
                EscapableHandleScope handle_scope(self->m_isolationScope);
                
                //-------------------------------------------------
                //  Firstly, make sure that this class wasn't marked as abstract
                //-------------------------------------------------
                
                if (self->m_isAbstract)
                {
                    std::stringstream io;
                    io << "Could not create an instance of the class " << TypeId<typename TypeResolver<TClass>::type >().name() << " since it was marked as abstract class." << std::endl;
#if V8BRIDGE_DEBUG
                    io << "In order to allow the class instantiation, you should call the NativeClass<TClass>::declareAsAbstract with false argument.";
#endif
                    self->m_isolationScope->ThrowException(
                                                           v8::Exception::TypeError(String::NewFromUtf8(self->m_isolationScope, io.str().c_str()))
                    );
                    return;
                }
                
                //-------------------------------------------------
                //  We got two cases to deal with.
                //      - The first one is in which we're trying to instansiate plain new object (e.g. from JS)
                //          In this case, we should allocate a new coresponding CPP object (i.e. "var p = new Point(x, y);" in JS will allocate new CPP Point).
                //      - The second one is case in which we're trying to Convert Existing CPP Object to JS (i.e. by using NativeToJs interface).
                //          in this case we still should create a new JS object, but We Shouldn't Allocate a new CPP object but using the Existing one.
                //
                //  As a result, we can't forward the constructor directly to the CPP ctor but create a constructor that discrimiate between these cases.
                //  The solution that came up is to pass the native instance as an External value when using NativeToJs. Here, we should check whether or not we've recevived as ctor args only 1 arg of type External.
                //  if so, this is NativeToJs. Otherwise - we're dealing with new instansiation.
                //
                //  For more details, see: http://create.tpsitulsa.com/blog/2009/01/29/v8-objects/
                //-------------------------------------------------
                External *externalInstance;
                TClass *instance;
                if (!info[0]->IsExternal())
                {
                    //-------------------------------------------------
                    //  Do we got at least one constructor?
                    //  If not, we shall use a default one
                    //-------------------------------------------------
                    
                    if (self->m_ctor->getOverloadsCount() < 1)
                    {
                        /* There's no constructor, so just create the object */
                        try
                        {
                            instance = new TClass();
                        }
                        catch (...)
                        {
                            delete instance;
                            throw;
                        }
                        
                        /* Save the binded C++ instance in the new created JS object */
                        info.This()->SetAlignedPointerInInternalField(info.This()->InternalFieldCount() - 2, (void *)instance);
                    }
                    else
                    {
                        /* Invoke the actual class constructor */
                        self->m_ctor->invoke(info);
                    
                        /* Get the TClass instance */
                        instance = (TClass *)info.This()->GetAlignedPointerFromInternalField(info.This()->InternalFieldCount() - 2);
                    }
                }
                else
                {
                    /* Get the v8::External instance */
                    externalInstance = External::Cast(*info[0]);
                    
                    /* Get the TClass instance */
                    instance = (TClass *)externalInstance->Value();
                }
                
                /* Save the TClass instance in the new created JS object */
                info.This()->SetAlignedPointerInInternalField(info.This()->InternalFieldCount() - 2, (void *)instance);
                
                /* Basic memory adjustment */
                self->m_isolationScope->AdjustAmountOfExternalAllocatedMemory(self->m_allocatedMemoryAdjustment);
               
                /* Save an pointer to "this" (self variable) since
                    we can't access it in the SetWeak() callback. */
                info.This()->SetAlignedPointerInInternalField(info.This()->InternalFieldCount() - 1, (void *)self);
                
                /* Create persistent object so we can make this object accessable outside the handle-scope ?*/
                v8::Persistent<v8::Object> handle(self->m_isolationScope, info.This());
                
                /* Setup weak connection */
                handle.SetWeak(instance, &NativeClass<TClass>::WeakObjectsDeletionCallback);
                handle.MarkIndependent();
                
                /* Expose to the class GC */
                if (self->m_customDtorHandler == NULL)
                {
                    self->m_gc->queue(instance); // we can not send null, otherwise even the default dtor wont be run, thus, the object won't be released.
                }
                else
                {
                    self->m_gc->queue(instance, self->m_customDtorHandler);
                }
            }
            
            //=======================================================================
            //  Private methods
            //=======================================================================
            
            template <typename TCtor, typename TCtorSignature>
            inline NativeClass<TClass> *_exposeCtor(TCtor callback, TCtorSignature signature)
            {
                this->m_ctor->template addOverload<TCtor, TCtorSignature>(callback, signature);
                return this;
            }
            
            template<typename TMethod>
            inline NativeClass<TClass> *_exposeMethod(std::string methodName, TMethod method, bool isStatic = false)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                //-------------------------------------------------
                //  Do we got a registered method with the same name
                //-------------------------------------------------
                
                TMethodsMap *map = isStatic ? this->m_staticMethods : this->m_methods;
                
                
                if (map->find(methodName) != map->end())
                {
                    //-------------------------------------------------
                    //  We do got a registered NativeFunction. In this case,
                    //  we should just add an overload.
                    //-------------------------------------------------
                    
                    (*map->find(methodName)).second->addOverload(method);
                    return this;
                }
                else
                {
                    //-------------------------------------------------
                    //  Create a new pair
                    //-------------------------------------------------
                    
                    /* New function instance */
                    NativeFunction *funcDecl = new NativeFunction(this->m_isolationScope);
                    
                    /* Add the initial overlaod */
                    funcDecl->addOverload(method);
                    
                    /* Create a shared pointer */
                    boost::shared_ptr<NativeFunction> adapter(funcDecl);
                    
                    if (!isStatic)
                    {
                        /* Store */
                        this->m_methods->insert(std::make_pair(methodName, adapter));
                        
                        /* Add to the instance template */
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()), funcDecl->getTemplate()->GetFunction());
                    }
                    else
                    {
                        /* Store */
                        this->m_staticMethods->insert(std::make_pair(methodName, adapter));
                        
                        /* Add to the global template */
                        this->getTemplate()
                        ->Set(String::NewFromUtf8(this->m_isolationScope, methodName.c_str()), funcDecl->getTemplate()->GetFunction());
                    }
                }
                
                return this;
            }
            
            template<typename TGetter>
            inline NativeClass<TClass> *_exposePropertyAccessor(std::string propertyName, TGetter getter, std::string getterMethodName = "", bool isStatic = false)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                //-------------------------------------------------
                //  Do we got a registered accesso with the same name
                //-------------------------------------------------
                
                TMethodsMap *map = isStatic ? this->m_staticAccessors : this->m_accessors;
                
                //-------------------------------------------------
                //  Create a new pair
                //-------------------------------------------------
                
                /* New function instance */
                NativeFunction *getterMethod = new NativeFunction(this->m_isolationScope);
                
                /* Add the initial overlaod */
                getterMethod->addOverload(getter);
                
                if (!isStatic)
                {
                    /* Getter method name */
                    if (!getterMethodName.empty())
                    {
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                    else
                    {
                        /* Just create a temp name in order to store the method instance in the map */
                        getterMethodName = "__get_accessor_" + propertyName;
                    }
                }
                else
                {
                    /* Getter method name */
                    if (!getterMethodName.empty())
                    {
                        this->getTemplate()
                        ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                    else
                    {
                        /* Just create a temp name in order to store the method instance in the map */
                        getterMethodName = "__get_accessor_" + propertyName;
                    }
                }
                
                /* Create a shared pointer */
                boost::shared_ptr<NativeFunction> adapter(getterMethod);
                
                /* Store */
                map->insert(std::make_pair(getterMethodName, adapter));
                
                if (!isStatic)
                {
                    /* Add to the instance template */
                    this->getTemplate()
                    ->InstanceTemplate()
                    ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ Local<FunctionTemplate>(),
                                          /* PropertyAttributes: */ReadOnly
                                          );
                }
                else
                {
                    /* Add to the instance template */
                    this->getTemplate()
                    ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ Local<FunctionTemplate>(),
                                          /* PropertyAttributes: */ReadOnly
                                          );
                }
                
                return this;
            }
            
            inline NativeClass<TClass> *_exposePropertyAccessor(std::string propertyName, NativeFunction *getterMethod, std::string getterMethodName = "", bool isStatic = false)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                //-------------------------------------------------
                //  Do we got a registered accesso with the same name
                //-------------------------------------------------
                
                TMethodsMap *map = isStatic ? this->m_staticAccessors : this->m_accessors;
                assert(map->find(propertyName) == map->end());
                
                //-------------------------------------------------
                //  Create a new pair
                //-------------------------------------------------
                
                /* Getter method name */
                if (!getterMethodName.empty())
                {
                    if (isStatic)
                    {
                        this->getTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                    else
                    {
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                }
                else
                {
                    /* Just create a temp name in order to store the method instance in the map */
                    getterMethodName = "__get_accessor_" + propertyName;
                }
                
                /* Store */
                boost::shared_ptr<NativeFunction> getterAdapter(getterMethod);
                map->insert(std::make_pair(getterMethodName, getterAdapter));
                
                if (!isStatic)
                {
                    /* Add to the instance template */
                    this->getTemplate()
                        ->InstanceTemplate()
                        ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ Local<FunctionTemplate>(),
                                          /* PropertyAttributes: */ReadOnly
                                          );
                }
                else
                {
                    /* Add to the instance template */
                    this->getTemplate()
                        ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ Local<FunctionTemplate>(),
                                          /* PropertyAttributes: */ReadOnly
                                          );
                }
            }
            
            template<typename TGetter, typename TSetter>
            inline NativeClass<TClass> *_exposePropertyAccessor(std::string propertyName, TGetter getter, TSetter setter, std::string getterMethodName = "", std::string setterMethodName = "", bool isStatic = false)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                //-------------------------------------------------
                //  Do we got a registered accesso with the same name
                //-------------------------------------------------
                
                TMethodsMap *map = isStatic ? this->m_staticAccessors : this->m_accessors;
                assert(map->find(propertyName) == map->end());
                
                //-------------------------------------------------
                //  Create a new pair
                //-------------------------------------------------
                
                /* New function instances */
                NativeFunction *getterMethod = new NativeFunction(this->m_isolationScope);
                NativeFunction *setterMethod = new NativeFunction(this->m_isolationScope);
                
                /* Add the initial overlaod */
                getterMethod->addOverload(getter);
                setterMethod->addOverload(setter);
                
                /* Getter method name */
                if (!getterMethodName.empty())
                {
                    if (!isStatic)
                    {
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                    else
                    {
                        this->getTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                }
                else
                {
                    /* Just create a temp name in order to store the method instance in the map */
                    getterMethodName = "__get_accessor_" + propertyName;
                }
                
                /* Setter method name */
                
                if (!setterMethodName.empty())
                {
                    if (!isStatic)
                    {
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, setterMethodName.c_str()), setterMethod->getTemplate());
                    }
                    else
                    {
                        this->getTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, setterMethodName.c_str()), setterMethod->getTemplate());
                    }
                }
                else
                {
                    /* Just create a temp name in order to store the method instance in the map */
                    setterMethodName = "__set_accessor_" + propertyName;
                }
                
                /* Create a shared pointer */
                boost::shared_ptr<NativeFunction> getterAdapter(getterMethod);
                boost::shared_ptr<NativeFunction> setterAdapter(setterMethod);
                
                /* Store */
                map->insert(std::make_pair(getterMethodName, getterAdapter));
                map->insert(std::make_pair(setterMethodName, setterAdapter));
                
                /* Add to the instance template */
                if (!isStatic)
                {
                    this->getTemplate()
                        ->InstanceTemplate()
                        ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ setterMethod->getTemplate()
                                          );
                }
                else
                {
                    this->getTemplate()
                        ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ setterMethod->getTemplate()
                                          );
                }
                
                return this;
            }
            
            
            inline NativeClass<TClass> *exposePropertyAccessor(std::string propertyName, NativeFunction *getterMethod, NativeFunction *setterMethod, std::string getterMethodName = "", std::string setterMethodName = "", bool isStatic = false)
            {
                HandleScope handle_scope(this->m_isolationScope);
                
                //-------------------------------------------------
                //  Do we got a registered accesso with the same name
                //-------------------------------------------------
                
                TMethodsMap *map = isStatic ? this->m_staticAccessors : this->m_accessors;
                assert(map->find(propertyName) == map->end());
                
                //-------------------------------------------------
                //  Create a new pair
                //-------------------------------------------------
                
                /* Getter method name */
                if (!getterMethodName.empty())
                {
                    if (!isStatic)
                    {
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                    else
                    {
                        this->getTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, getterMethodName.c_str()), getterMethod->getTemplate());
                    }
                }
                else
                {
                    /* Just create a temp name in order to store the method instance in the map */
                    getterMethodName = "__get_accessor_" + propertyName;
                }
                
                
                if (!setterMethodName.empty())
                {
                    if (!isStatic)
                    {
                        this->getTemplate()
                            ->InstanceTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, setterMethodName.c_str()), setterMethod->getTemplate());
                    }
                    else
                    {
                        this->getTemplate()
                            ->Set(String::NewFromUtf8(this->m_isolationScope, setterMethodName.c_str()), setterMethod->getTemplate());
                    }
                }
                else
                {
                    /* Just create a temp name in order to store the method instance in the map */
                    setterMethodName = "__set_accessor_" + propertyName;
                }
                
                
                /* Store */
                boost::shared_ptr<NativeFunction> getterAdapter(getterMethod);
                boost::shared_ptr<NativeFunction> setterAdapter(setterMethod);
                
                map->insert(std::make_pair(getterMethodName, getterAdapter));
                map->insert(std::make_pair(setterMethodName, setterAdapter));
                
                /* Add to the instance template */
                if (!isStatic)
                {
                    this->getTemplate()
                        ->InstanceTemplate()
                        ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ setterMethod->getTemplate()
                                          );
                }
                else
                {
                    this->getTemplate()
                        ->SetAccessorProperty(
                                          /* name: */   String::NewFromUtf8(this->m_isolationScope, propertyName.c_str()),
                                          /* getter: */ getterMethod->getTemplate(),
                                          /* setter: */ setterMethod->getTemplate()
                                          );
                }
                
            }
        };
    }
}

#endif