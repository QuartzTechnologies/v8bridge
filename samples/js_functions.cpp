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
 * This file provides a simple demonstration of defining JS function and accessing it by C++ using the Userland API.
 * In this demo, we're declaring a simple add function in JS and then invoking it from C++.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

using namespace v8;
using namespace v8::bridge;

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    HandleScope handle_scope(engine->getActiveIsolationScope());
    
    /* Create a new "JS class" */
    std::stringstream io;
    io << "function add(x, y) {"                << std::endl;
    io << " return x + y;"                      << std::endl;
    io << "}"                                   << std::endl;
    io << "function multiply(x, y) {"           << std::endl;
    io << " if (typeof(y) == 'undefined') {"    << std::endl;
    io << "     y = 2;"                         << std::endl;
    io << " }"                                  << std::endl;
    io << " return x * y;"                      << std::endl;
    io << "}"                                   << std::endl;
    
    
    engine->execute(io.str(), /* fileName: */ "add.js");
    
    /** To access the JS function, we should use the UserlandFunction class which
     provides us basic interface for interaction with the defined class. **/
    UserlandFunction *addFunction = new UserlandFunction(engine->getActiveIsolationScope(), engine->getFromGlobalScope("add"));
    UserlandFunction *multiplyFunction = new UserlandFunction(engine->getActiveIsolationScope(), engine->getFromGlobalScope("multiply"));
    
    /* Invoke the function */
    int result = addFunction->invoke<int>(1, 1);        // JS add(1, 1): 2
    result = multiplyFunction->invoke<int>(result);     // JS multiply(2): 4
    
    std::cout << "Result: " << result << std::endl;
    
    /* Free */
    delete addFunction;
    delete multiplyFunction;
    delete engine;
    
    /* Done. */
    return 0;
}
