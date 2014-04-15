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
 * This file provides a simple demonstration of accepting directly V8 args info.
 * In this demo, we're declaring a simple add function. However, we're not using the Conversion API
 * to convert the args into native types. We're accepting raw-V8 args and adding all of them.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

using namespace v8;
using namespace v8::bridge;

int add(const FunctionCallbackInfo<Value> &info)
{
    if (info.Length() == 0)
    {
        throw std::runtime_error("The add function must receive at least one argument.");
    }
    
    int result = 0;
    for (int i = 0; i < info.Length(); i++)
    {
        if (!info[i]->IsInt32())
        {
            throw std::runtime_error("All of the function passed arguments should be integers.");
        }
        
        result += info[i]->Int32Value();
    }
    
    return result;
}

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Create the add function */
    NativeFunction *addFunction = new NativeFunction(engine->getActiveIsolationScope());
    addFunction->addOverload(add);
    engine->exposeFunction(addFunction, "add");
    
    /* Execute! */    
    std::cout << "add(1): " << engine->eval<int>("add(1)") << std::endl;
    std::cout << "add(1, 1): " << engine->eval<int>("add(1, 1)") << std::endl;
    std::cout << "add(1, 2): " << engine->eval<int>("add(1, 2)") << std::endl;
    std::cout << "add(1, 2, 3): " << engine->eval<int>("add(1, 2, 3)") << std::endl;
    std::cout << "add(1, 2, 2, 3): " << engine->eval<int>("add(1, 2, 2, 3)") << std::endl;
    
    /* Free */
    delete addFunction;
    delete engine;
    
    /* Done. */
    return 0;
}
