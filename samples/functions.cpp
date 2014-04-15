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
 * This file provides a simple functions usage demo.
 * In this demo, we're registering 3 functions:
 *      - add: A simple function that receives two numbers (int, int) and returns the addition result (int).
 *      - multiply: A simple multiply function. This function gots two overloads,
 *          that we're exposing to JS. multiply(x) which multiply x by 2 and multiply(x, y) which multiplys x by y.
 *      - echo: A simple function thats echo the given string to stdout. This function, too, gots two overloads.
 *          The first overloads accepts only a string and returns void. The scond one accetps string and int,
 *              concat them in the output and returns void.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

/* Simple "add" function */
int add(int x, int y) { return x + y; }

/* Simple multiply function. We're declaring two overloads that we'll expose to JS. */
int multiply(int x) { return x * 2; }
int multiply(int x, int y) { return x * y; }

/* Simple echo functions. The second function,
    though its got a different name, will be an overload of "echo". */
void echo(std::string str)
{
    std::cout << str << std::endl;
}

void echo_with_number(std::string str, int n)
{
    std::cout << str << " " << n << std::endl;
}

int main(int argc, const char * argv[])
{
    using namespace v8::bridge;
    
    /* Create the scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Create endpoints for our functions */
    NativeFunction *addFunction = new NativeFunction(engine->getActiveIsolationScope());
    NativeFunction *multiplyFunction = new NativeFunction(engine->getActiveIsolationScope());
    NativeFunction *echoFunction = new NativeFunction(engine->getActiveIsolationScope());
    
    /* Add overloads */
    
    // Add:
    addFunction->addOverload(add);
    
    // Multiply: Since the functions in C++ got the same name,
    // we must specify the function signature.
    multiplyFunction->addOverload<int (int)>(multiply)
                    ->addOverload<int (int, int)>(multiply);
    
    // Echo: there's no problem at all to connect two C++ functions with
    // diffrent name as overloads for the same JS function.
    echoFunction->addOverload(echo)
                ->addOverload(echo_with_number);
    
    /* Register at the global scope */
    engine->exposeFunction(addFunction, "add");
    engine->exposeFunction(multiplyFunction, "multiply");
    engine->exposeFunction(echoFunction, "echo");
    
    /* Execute! */
    std::stringstream io;
    io << "var x = 2;" << std::endl;
    io << "var y = 3;" << std::endl;
    io << "echo('========================================');" << std::endl;
    io << "echo('x + y (calling add(x, y)):', add(x, y));" << std::endl;
    io << "echo('========================================');" << std::endl;
    io << "echo('x * 2 (calling multiply(x)):', multiply(x));" << std::endl;
    io << "echo('========================================');" << std::endl;
    io << "echo('x * y (calling multiply(x, y)):', multiply(x, y));" << std::endl;
    
    engine->execute(io.str(), /* fileName: */ "functions.js");
    
    /* Free */
    delete addFunction;
    delete multiplyFunction;
    delete echoFunction;
    delete engine;
    
    /* Done. */
    return 0;
}
