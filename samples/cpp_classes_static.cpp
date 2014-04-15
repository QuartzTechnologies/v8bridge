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
#include <fstream>

#define V8BRIDGE_DEBUG 1
#include <v8bridge/v8bridge.hpp>

using namespace v8;
using namespace v8::bridge;

/* Declare a file-utils class */
class FileUtils
{
public:
    static bool FileExists(std::string const filePath)
    {
        std::ifstream f(filePath.c_str());
        if (f.good()) {
            f.close();
            return true;
        } else {
            f.close();
            return false;
        }
    };
    
    static std::string Read(std::string const filePath)
    {
        std::ifstream f(filePath.c_str());
        if (f.good()) {
            return "";
        }
        
        std::string result((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
        return result;
    };
};

void print(std::string text)
{
    std::cout << text << std::endl;
}

int main(int argc, const char * argv[])
{
    /* Create a scripting engine */
    ScriptingEngine *engine = new ScriptingEngine();
    
    /* Expose a sample print function (Since we're not relaying on addons in the sample) */
    NativeFunction *printFunction = new NativeFunction(engine->getActiveIsolationScope());
    printFunction->addOverload(print);
    engine->exposeFunction(printFunction, "print");
    
    /* Create a new NativeClass which will be our endpoint for a native C++ class. */
    NativeClass<FileUtils> *fileUtilsClass = new NativeClass<FileUtils>(engine->getActiveIsolationScope());
    
    /* Expose some static methods */
    fileUtilsClass->exposeStaticMethod("FileExists", &FileUtils::FileExists)
                ->exposeStaticMethod("Read", &FileUtils::Read);
    
    /* Expose the car to JS global scope. */
    engine->exposeClass(fileUtilsClass);
    
    /* Execute! */
    std::stringstream io;
    io << "var filePath = './foo.txt';"                                             << std::endl;
    io << "if (FileUtils.FileExists(filePath)) {"                                   << std::endl;
    io << " var contents = FileUtils.Read(filePath);"                               << std::endl;
    io << " print('File Path: ' + filePath + '. Contents:');"                       << std::endl;
    io << " print(contents);"                                                       << std::endl;
    io << "} else {"                                                                << std::endl;
    io << " print('The requested file path (' + filePath + ') does not exists.');"  << std::endl;
    io << "}"                                                                       << std::endl;
    
    
    
    engine->execute(io.str());
    
    /* Free */
    delete printFunction;
    delete fileUtilsClass;
    delete engine;
    
    /* Done. */
    return 0;
}
