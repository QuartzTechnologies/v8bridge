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
 * This file provides a simple demonstration of interaction with JS-defined variables from C++.
 * In this demo, we're declaring some JS variables from different types and them receiving them
 * by vairous v8bridge APIs
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

int main(int argc, const char * argv[])
{
    using namespace v8::bridge;
    
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Execute simple script which contains some variable declarations in the global scope */
    std::stringstream io;
    io << "var intVar          = 10;" << std::endl;
    io << "var numberVar       = 10.5;" << std::endl;
    io << "var stringVar       = 'Hello, World!';" << std::endl;
    io << "var boolVar         = false;" << std::endl;
    io << "var nullVar         = null;" << std::endl;
    io << "var undefinedVar    = undefined;" << std::endl;
    io << "var arrayVar        = [1, 2, 3];" << std::endl;
    io << "var objectVar       = { 'foo': 1, 'bar': 2 };" << std::endl;
    io << "var callbackVar     = function() { };" << std::endl;
    
    engine->execute(io.str());
    
    /*
     * Access each variable. Since the variables was defined
     * in the global scope, we can easly access them using
     * ScriptingEngine::getFromGlobalScope and ScriptingEngine::getValueFromGlobalScope<TType>.
     *
     * Note that you should may use getFromGlobalScope<TType> only when you're sure that the given variable
     * is in your desired type, otherwsie an runtime_error will raise.
     */
    
    /* Lets start by retriving variables using the Conversion API: */
    
    // Int:
    int intVar = engine->getValueFromGlobalScope<int>("intVar");
    std::cout << "intVal: " << intVar << std::endl;
    
    // Number (double/float):
    double numberVar = engine->getValueFromGlobalScope<double>("numberVar");
    std::cout << "numberVar: " << numberVar << std::endl;
    
    // String:
    std::string stringVar = engine->getValueFromGlobalScope<std::string>("stringVar");
    std::cout << "stringVar: " << stringVar << std::endl;
    
    // Bool:
    bool boolVar = engine->getValueFromGlobalScope<bool>("boolVar");
    std::cout << "boolVar: " << boolVar << std::endl;
    
    /* To retrive null and undefined, we must retrive the Handle<Value> */
    
    // Note: since we're going to interact with raw handles,
    // and we're in main, we must create a new handle scope.
    //
    // Note 2: We don't need to open a scope when interacting with raw values
    // in functions and methods that was exposed to JS since v8bridge
    // opens one for us before triggering our function.
    HandleScope handle_scope(engine->getActiveIsolationScope());
    
    // Null:
    Handle<Value> nullVar = engine->getFromGlobalScope("nullVar");
    std::cout << "nullVar (is null?): " << (nullVar->IsNull()) << std::endl;
    
    // Undefined:
    Handle<Value> undefinedVar = engine->getFromGlobalScope("undefinedVar");
    std::cout << "undefinedVar (is undefined?): " << (undefinedVar->IsUndefined()) << std::endl;
    
    /*
     * Array:
     * There're two ways to interact with arrays:
     *  1. JS arrays are been converted automaticly to list. So we can use the
     *  conversion API to convert the Handle<Array> to a native list type. The con
     *  of this action is that the array must contain a specific type (e.g. strings only, ints only). 
     *  2. We can receive the array as raw Handle<Array>. To make things simpler
     *  we can avoid the type-verification by requesting from the Conversion API to get Handle<Array>.
     *  this will make sure that we're dealing with an array (and not with string or undefined variable, for instance).
     */
#if 0 // std::list<T> approch:
    std::list<int> arrayVar = engine->getValueFromGlobalScope<std::list<int> >("arrayVar");
    std::cout << "arrayVar (size: " << arrayVar.size() << "): [";
    for (int i = 0; i < arrayVar.size(); i++)
    {
        if (i < arrayVar.size() - 1)
            std::cout << i << ", ";
        else
            std::cout << i;
    }
    std::cout << "]" << std::endl;
#elif 1 // Handle<Array>
    Handle<Array> arrayVar = engine->getHandleFromGlobalScope<Handle<Array> >("arrayVar");
    String::Utf8Value arrayVarStringRep(arrayVar); // We're using v8::String::Utf8Value to evaluate the array contents into a string
    std::cout << "arrayVar (size: " << arrayVar->Length() << "):"
    << "[" << *arrayVarStringRep << "]" << std::endl;
#else // Handle<Value> raw parsing
    Handle<Value> arrayVar = engine->getFromGlobalScope("arrayVar");
    assert(arrayVar->IsArray());
    String::Utf8Value arrayVarStringRep(arrayVar);
    std::cout << "arrayVar (size: " << Handle<Array>::Cast(arrayVar)->Length() << "):"
   
    << "[" << *arrayVarStringRep << "]" << std::endl;
#endif
    
    /* Free */
    delete engine;
    
    /* Done. */
    return 0;
}
