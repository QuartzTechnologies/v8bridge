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
 * This file provides a simple demonstration of returning raw V8 handle.
 * In this demo, we're declaring a simple number_by_type function. In this function, we're not using the
 * Conversion API to convert between native types to V8 but returning Handle<Value> directly. This approch
 * allows us to return different value types.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

using namespace v8;
using namespace v8::bridge;

Handle<Value> number_by_type(std::string type)
{
    if (type == "int") {
        return Int32::New(Isolate::GetCurrent(), 1);
    }
    if (type == "string") {
        return String::NewFromUtf8(Isolate::GetCurrent(), "one");
    }
    
    return Undefined(Isolate::GetCurrent());
}

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Create the add function */
    NativeFunction *function = new NativeFunction(engine->getActiveIsolationScope());
    function->addOverload(number_by_type);
    engine->exposeFunction(function, "number_by_type");
    
    /* Execute! */
    std::cout << "number_by_type('int'): " << engine->eval<int>("number_by_type('int')") << std::endl;
    std::cout << "number_by_type('string'): " << engine->eval<std::string>("number_by_type('string')") << std::endl;
    
    /* Free */
    delete function;
    delete engine;
    
    /* Done. */
    return 0;
}
