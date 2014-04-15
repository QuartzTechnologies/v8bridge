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
 * This file provides a simple demonstration of defining JS "class" and accessing it by C++ using the Userland API.
 * In this demo, we're declaring a simple Car JS class. Then, by using UserlandClass we're creating a new instance,
 * calling it's method(s) and changing its ivar value.
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
    io << "var Car = (function () {"                                << std::endl;
    io << " function Car(color) {"                                  << std::endl;
    io << "     this.color = color;"                                << std::endl;
    io << " };"                                                     << std::endl;
    io << " Car.prototype.getColor = function () {"                 << std::endl;
    io << "     return this.color;"                                 << std::endl;
    io << " };"                                                     << std::endl;
    io << " Car.prototype.setColor = function (color) {"            << std::endl;
    io << "     this.color = color;"                                << std::endl;
    io << " };"                                                     << std::endl;
    io << " Car.prototype.drive = function () {"                    << std::endl;
    io << "     return 'I\\'m driving in ' + this.color + ' car!';" << std::endl;
    io << " };"                                                     << std::endl;
    io << " return Car;"                                            << std::endl;
    io << "})();"                                                   << std::endl;
    
    engine->execute(io.str(), /* fileName: */ "car.js");
    
    /** To access the JS class, we should use the UserlandClass class which
     provides us basic interface for interaction with the defined class.
     
     Note that UserlandClass ctor requires an Handle<Function> (there's an overload for Handle<Value>)
     which represents the JS class ctor. To get it, as shown below, we can use the ScriptingEngine::getFromGlobalScope method,
     which retrives an handle that's been stored on the global scope. The handle we wish to request is the class representing variable - Car. **/
    UserlandClass *carJsClass = new UserlandClass(engine->getActiveIsolationScope(), engine->getFromGlobalScope("Car"));
    
    /** Now, lets create a new instance of the JS class. The C++ type that will hold the created type
     is UserlandInstance. UserlandInstance provides us simple interface to execute instance-related actions.
     
     Note that the created instance is been stored as boost::shared_ptr in UserlandClass and will be released when
     the UserlandClass object will be disposed. */
    UserlandInstance *carInstance = carJsClass->newInstance( /* color: */ "Red");
    
    // Print the current driving state (driving with red car, because of JS ctor)
    std::cout << carInstance->invoke<std::string>("drive") << std::endl;
    
    // Set the car color using the defined setter
    carInstance->invoke<void>("setColor", "Blue");
    
    // Print the current driving state (driving with blue car)
    std::cout << carInstance->invoke<std::string>("drive") << std::endl;
    
    // Set the car color by (in a very hacky way!) accessing the class member directly
    carInstance->setValue("color", "Yellow");
    
    // Print the current driving state (driving with yellow car)
    std::cout << carInstance->invoke<std::string>("drive") << std::endl;
    
    /* Free */
    delete carJsClass;
    delete engine;
    
    /* Done. */
    return 0;
}
