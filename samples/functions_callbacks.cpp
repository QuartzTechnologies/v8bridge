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
 * This is an sample file for v8bridge API interaction.
 *
 * This file provides a simple demonstration of invoking a JS callback from C++.
 * In this demo, we're declaring a simple filter_int_array (or filter_array) function, which aim is to
 * filter an array using a given callback. The supplied callback received from JS.
 * We're firing the callback using v8::bridge::invoke_v8_handle (or invoke_v8_handle_raw) set of functions.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

using namespace v8;
using namespace v8::bridge;

/* Declare a function that filters an array using a callback (which is been received by JS): */

#if 1 // Int-only implementation using the Conversion API (std::list<T> representing the Array)
std::list<int> filter_int_array(std::list<int> array, Handle<Function> callback)
{
    std::list<int> filteredArray;
    
    for (std::list<int>::iterator it = array.begin(); it != array.end(); ++it)
    {
        bool result = invoke_v8_handle<bool>(Isolate::GetCurrent(), callback, *it);
        if (result)
        {
            filteredArray.push_back(*it);
        }
    }
    
    return filteredArray;
}
#else // General implementation of array_filter example:
Handle<Value> filter_int_array(Handle<Array> array, Handle<Function> callback)
{
    Handle<Array> result = Array::New(Isolate::GetCurrent());//, (int)array->Length());
    int resultArrayIndex = 0;
    for (int i = 0; i < array->Length(); i++)
    {
        Handle<Value> callbackResult = invoke_v8_handle_raw(Isolate::GetCurrent(), callback, array->Get(Int32::New(Isolate::GetCurrent(), i)));
        /* Could also do it using the conversion API:
         bool callbackResult = invoke_v8_handle<bool>(Isolate::GetCurrent(), callback, array->Get(Int32::New(Isolate::GetCurrent(), i)));
        */
        
        if (callbackResult->IsTrue())
        {
            result->Set(resultArrayIndex++, array->Get(Int32::New(Isolate::GetCurrent(), i)));
        }
        
    }
    
    return result;
}
#endif

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Create the filter function */
    NativeFunction *filterFunction = new NativeFunction(engine->getActiveIsolationScope());
    
    /* Create the filter callback function */
    NativeFunction *filterCallbackFunction = new NativeFunction(engine->getActiveIsolationScope());
    filterFunction->addOverload(filter_int_array);
    engine->exposeFunction(filterFunction, "filter_int_array");
    
    
    /* Execute! */
    std::stringstream io;
    io << "var array = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];";
    io << "filter_int_array(array, function(a) {" << std::endl;
    io << " return a >= 5;" << std::endl;
    io << "});" << std::endl;
    
    HandleScope handle_scope(engine->getActiveIsolationScope());
    String::Utf8Value output(engine->v8Eval(io.str()));
    std::cout << *output << std::endl;
    
    /* Free */
    delete filterFunction;
    delete engine;
    
    /* Done. */
    return 0;
}
