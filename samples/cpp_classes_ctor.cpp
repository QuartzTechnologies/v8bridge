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
 * In this demo, we're defining 4 constructors in the Car class. Then, we're exposing them to JS and using them.
 */

#include <iostream>
#include <sstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

/* Macro shortcut that evaluates a given script and echo its result */
#define JS_PRINT_HANDLE(jsCode)     do \
                                    { \
                                        String::Utf8Value utf8(engine->v8Eval(jsCode)); \
                                        std::cout << *utf8 << std::endl; \
                                    } while (false);

using namespace v8;
using namespace v8::bridge;

/* Declare a simple class */
class Car
{
public:
    Car() : m_color(""), m_km(0) { };
    Car(std::string color) : m_color(color), m_km(0) { };
    Car(int km) : m_color(""), m_km(km) { };
    Car(std::string color, int km) : m_color(color), m_km(km) { };
    
    Car(const FunctionCallbackInfo<Value> &info)
    {
        /* This constructor requires at least one argument.
         Calling Signature:
         Car(string format [, int km [, int ... ]] )
         
         It can reminds us the add() method in the functions_v8_args.cpp example.
         */
        
        if (info.Length() == 0)
        {
            throw std::runtime_error("The add function must receive at least one argument.");
        }
        
        if (!JsToNative(Isolate::GetCurrent(), this->m_color, info[0]))
        {
            throw std::runtime_error("The first constructor argument must be a string (car color).");
        }
        
        this->m_km = 0;
        for (int i = 1; i < info.Length(); i++)
        {
            if (!info[i]->IsInt32())
            {
                throw std::runtime_error("All of the function passed arguments should be integers.");
            }
            
            this->m_km += info[i]->Int32Value();
        }
    }
    
    std::string getColor()
    {
        return this->m_color;
    }
    
    int getKm()
    {
        return this->m_km;
    }
private:
    std::string m_color;
    int m_km;
};

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Setup our endpoint */
    NativeClass<Car> *carClass = new NativeClass<Car>(engine->getActiveIsolationScope());
    
    // Properties (We're exposing read-only properties)
    carClass->exposePropertyAccessor("color", &Car::getColor)
            ->exposePropertyAccessor("km", &Car::getKm); 
    
    /* Expose the class ctors. Since we can't receive from C++ the singature of a ctor (or dtor)
     we should use an alternative approch. Based on boost python binding approch, I've created a
     ctor types-list which is representing a ctor singature. You may send it to NativeClass<TClass>::exposeCtor()
     to declare a class.
     
     Examples:
        exposeCtor(ctor<>()) - empty (default) ctor. Note: you should not declare it unless you got another constructor overload.
        exposeCtor(ctor<int>()) - ctor that accepts an int.
        exposeCtor(ctor<int, int>()) - ctor that accepts two ints.
     */
    carClass->exposeCtor(ctor<>()) // Car()
            ->exposeCtor(ctor<std::string>()) // Car(std::string color)
            ->exposeCtor(ctor<int>()) // Car(int km)
            ->exposeCtor(ctor<std::string, int>()) // Car(std::string color, int km)
            ->exposeCtor(ctor<const FunctionCallbackInfo<Value> &>()); // fallback ctor - Car(const FunctionCallbackInfo<Value> &info)
    
    /* Expose the car to JS global scope. */
    engine->exposeClass(carClass);
    
    /* Execute! */
    HandleScope handle_scope(engine->getActiveIsolationScope()); // JS_PRINT_HANDLE uses ScriptingEngine::v8Eval which requires an handle scope since it returns raw v8 values
    
    // Fires Car(). should print blank/zero values since color and km was not set.
    JS_PRINT_HANDLE("var car = new Car(); 'Color: ' + car.color + '; KM: ' + car.km;");
    std::cout << "=========================================" << std::endl;
    
    // Fires Car(std::string color). Should print 0 as KM since it was not set and "Red" as color.
    JS_PRINT_HANDLE("var car = new Car('Red'); 'Color: ' + car.color + '; KM: ' + car.km;");
    std::cout << "=========================================" << std::endl;
    
    // Fires Car(int km). Should print blank color since it was not set and 10 as KM.
    JS_PRINT_HANDLE("var car = new Car(10); 'Color: ' + car.color + '; KM: ' + car.km;");
    std::cout << "=========================================" << std::endl;
    
    // Fires Car(std::string color, int km). Should print "Red" as color and 10 as KM.
    JS_PRINT_HANDLE("var car = new Car('Red', 10); 'Color: ' + car.color + '; KM: ' + car.km;");
    std::cout << "=========================================" << std::endl;
    
    // Fires Car(const FunctionCallbackInfo<Value> &info). Should print "Red" as color and 16 as KM.
    JS_PRINT_HANDLE("var car = new Car('Red', 10, 1, 2, 3); 'Color: ' + car.color + '; KM: ' + car.km;");
    std::cout << "=========================================" << std::endl;
    
    /* Free */
    delete carClass;
    delete engine;
    
    /* Done. */
    return 0;
}
                    
#undef JS_PRINT_HANDLE
