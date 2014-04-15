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
 * This file provides a simple demonstration of binding a C++ class to JS.
 * In this demo, we're declaring a simple Car with one ivar (m_color) and three methods - getter, setter and standard method.
 * Then, we're expsoing the Car class using v8bridge NativeClass interface.
 * Finally, we're creating in JS a new instance of Car, setting its color using the "color" property and invoking drive().
 * 
 * Note: when "delete carClass;" is been called, the "car" instance that we've defined in JS is been removed
 * by v8bridge internal GC. For more details on why is this happing and why we need this, see internal_gc.hpp.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

using namespace v8;
using namespace v8::bridge;

/* Declare a simple class */
class Car
{
public:
    Car() { };
    
    void setColor(std::string color)
    {
        this->m_color = color;
    }
    
    std::string getColor()
    {
        return this->m_color;
    }
    
    void drive()
    {
        std::cout << "** I'm driving a " << this->getColor() << " car! **" << std::endl;
    }
private:
    std::string m_color;
};

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Create a new NativeClass which will be our endpoint for a native C++ class.
     The TClass template arg representing the class that you wish to expose.
     When exposing a class, it's been automaticly added to the Conversion API. */
    NativeClass<Car> *carClass = new NativeClass<Car>(engine->getActiveIsolationScope());
    
    /* We want to expose the "getColor" and "setColor" getter/setter
        and the drive method. Since the constructor is default, we don't need
     to explicity expose it.
     
     Note: If you don't want to allow to use the default constructor, you got two options:
     1. You can use the exposeCtor with a ctor that requires an arguments. In this case,
     v8bridge won't allow the non-args ctor.
     2. You can use the declareAsAbstract() method to declare the class as abstract.
     */
    
    // drive - expose a method:
    carClass->exposeMethod("drive", &Car::drive);
    
    // getColor and setColor - expose as property accessor:
    carClass->exposePropertyAccessor("color", &Car::getColor, &Car::setColor);
    
    /* Expose the car to JS global scope.
     NOTE: just like in NativeFunction, you MUST call this method,
     and you MUST do it AFTER FINISHING TO CONFIGURE YOUR ENDPOINT. */
    engine->exposeClass(carClass);
    
    /* Execute! */
    std::stringstream io;
    io << "var car = new Car();" << std::endl;
    io << "car.color = 'red';" << std::endl;
    io << "car.drive();";
    
    engine->execute(io.str());
    
    /* Free */
    delete carClass;
    delete engine;
    
    /* Done. */
    return 0;
}
