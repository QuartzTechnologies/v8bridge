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

#ifndef v8bridge_internal_gc_hpp
#define v8bridge_internal_gc_hpp

#include <v8bridge/detail/prefix.hpp>

namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            template <class TClass>
            struct GCGenericInstanceDestructor
            {
                inline static void dtor(void *ptr)
                {
                    TClass *instance = ptr ? static_cast<TClass *>(ptr) : NULL;
                    if (instance != NULL)
                    {
                        delete instance;
                    }
                }
            };
        }
        
        /**
         * Internal bridge Garbage Collector implementation.
         *
         * This implementation provides a "client-side" garbage collection routines,
         * in order to bypass some certion issues with V8's GC.
         *
         * The main issue that we're facing is that there's no guarantee that V8 GC will,
         * ever been executed, EVEN ON SHUT-DOWN, which will lead to memory leak.
         *
         * This implemementation provides a workaround which clears all un-dealed binded pointers.
         */
        class V8_DECL GC
        {
        public:
            GC(Isolate *isolate) : m_isolationScope(isolate), m_map(new TCleanupMap()) { }
            ~GC()
            {
                this->collect(); // collect and release any remaining instances on the cleanup list.
            }
            
            typedef void (*TDtor)(void *ptr);
            
            /**
             * Queue the given object instance in the internal GC clearup instances list.
             * For more details about this method, see the second overload above.
             *
             * Note that this method make use of the GCGenericInstanceDestructor<TType>::dtor destructor implementation
             * in order to remove the object.
             * This is the default and typical implementation for destructors, which only deletes the instance from the memoory.
             * Thus, this implementation will fire the instance class dtor.
             */
            template <class TType>
            inline void queue(TType *object)
            {
                this->queue(object, detail::GCGenericInstanceDestructor<TType>::dtor);
            }
            
            /**
             * Adds the given object to the internal GC cleanup instances list.
             * This operation will cause that when collect or ~GC (GC dtor) will be called
             * this object will be disposed.
             *
             * In order to dispose the object, the given TDtor callback will be executed.
             *
             * The typical usage for this class is in NativeClass<TClass>, when handling the class constructor,
             * so when we're creating a new JS instance and binding it to C++ instance,
             * we can register the created C++ instance for future cleanup.
             *
             * In this regards, please note that if you'd like to dispose the object
             * before executing GC::collect, you may call dequeue to remove the instance
             * from the GC instances queue list.
             */
            inline void queue(void *object, TDtor dtor)
            {
                this->m_map->insert( std::make_pair(object, dtor) );
            }
            
            /**
             * Removes the given object from the GC cleanup instances list.
             */
            inline void dequeue(void *object)
            {
                this->m_map->erase(object);
            }
            
            /**
             * Removes the given object from the GC cleanup instances list.
             */
            inline void disposeAndDequeue(void *object)
            {
                TCleanupMap::iterator pair = this->m_map->find(object);
                if (pair == this->m_map->end())
                {
                    return;
                }
                
                if (pair->second != NULL)
                {
                    (pair->second)(pair->first);
                }
                
                this->m_map->erase(object);
            }
            
            /**
             * Collect and release any instance in the GC cleanup list.
             *
             * Note that this method will invalidate all registered instances and make them NULL.
             * Thus, you may call this method only, and ONLY, when you're sure that no registered
             * instance is been used by JS.
             * 
             * To remove single object, use GC::dequeue.
             */
            inline void collect()
            {
                for (TCleanupMap::iterator it = this->m_map->begin(); it != this->m_map->end(); ++it)
                {
                    if (it->second != NULL) // has valid TDtor
                    {
                        (it->second)(it->first);
                    }
                }
                
                this->m_map->clear();
            }
        private:
            typedef std::map<void *, TDtor> TCleanupMap;
            
            TCleanupMap *m_map;
            Isolate *m_isolationScope;
        };
    }
}

#endif
