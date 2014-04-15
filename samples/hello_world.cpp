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
 * This file provides a simple "hello world" demo.
 * In this demo, we're registring our "hello" function, which is a
 * standard native C++ function that don't receives any argument and returns void,
 * to JS.
 *
 * Than, executing the function from JS script.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

void hello()
{
    std::cout << "Hello, World!" << std::endl;
}

int main(int argc, const char * argv[])
{
    using namespace v8::bridge;
    
    /* Firstly, we should create a new scripting engine.
        This will initialize V8 (isolate, context etc.)
        and will work as the interface for us to bind C++ classes with V8 */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Since we wish to create a JS function, we should use v8bridge NativeFunction
     class which gives us a simple interface to register native functions with JS. */
    NativeFunction *helloFunction = new NativeFunction(engine->getActiveIsolationScope());
    
    /* Add our "hello" function as an overload */
    helloFunction->addOverload(hello);
    
    /* Register the function in ScriptingEngine. This will expose the function
        as an endpoint in JS. Note that you MUST do it, and you MUST call it
        AFTER YOU'VE FINISHED CONFIGURED YOUR FUNCTION. */
    engine->exposeFunction(helloFunction, "hello");
    
    /* Execute! */
    engine->execute("hello();", /* fileName: */ "hello_world.js");
    
    /* Free */
    delete helloFunction;
    delete engine;
    
    /* Done. */
    return 0;
}
