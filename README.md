Welcome to bridge.js!
=====================

Introduction
---------

**bridge.js** (or v8bridge) is a C++ header-only library that extends Google V8 JavaScript engine which aims to provide simple interface to embed the v8 JavaScript engine in a C++ application (regardless of the application relation to the web).

Alternative/Related Works
---------
**bridge.js** is not the first approach to create a bridge between C++ and JavaScript.

Some popular projects, based on v8 engine, includes:

- [**node.js**](http://nodejs.org): One of the most popular projects today.
- [**Nasiu Scripting**](https://code.google.com/p/nasiu-scripting): One of my favorite projects. The **Conversion API** initially inspired by it.
- [**vu8**](https://github.com/tsa/vu8): The project goal is very similar to **bridge.js** and is one of the few bindings frameworks I've seen which uses templates to generate the bindings (as opposed to macros). If I wouldn't choose **bridge.js** and V8 wouldn't change it's whole API, I were choose this project.
- [**cproxyv8**](https://code.google.com/p/cproxyv8/).

In addition, you can browse more projects that relays on Mozila's Spidermonkey as its backend JS engine:

- [**libjspp**](https://code.google.com/p/libjspp/): One of my favorite projects and is very similar to bridge.js. Unfortunately, the project source code is not available anymore.
- [**Flusspferd**](https://github.com/Flusspferd/flusspferd).
- [**SpiderApe**](http://sourceforge.net/p/spiderape).

Objectives
---------

As you can see above, there're planty of awesome similar projects. So what is so special about bridge.js?
When looking for bindings library, I wished for a library that can handle native types, so I won't need to interact directly with V8 API in order to bind C++ functions to JS.
Moreover, I wanted a library that make it easy to bind **even existing** standard functions and classes, to JS, so I won't need to write specific implementations for dedicated only to JS (which results in code duplication, increased development and maintenance time etc.).

To sort things out, the main objectives of **bridge.js** are:

- Provides OOP interface to interact with JavaScript.
- Support any type of C++ Applications, regardless of thier relation to the web (i.e. games, complicated desktop programs etc.).
- Supports the latest V8 engine API.
- Allow to simply expose C++ functions and classes, **even existing**, without the need of callbacks.
- Work with native data types without being restricted to V8 handles (int instead of Handle<Int32>, std::string instead of Handle<String> etc.).
- Allow to specify functions and methods overloads (i.e. int multiply(x) and int multiply(x, y) which will be resolved by the parameters sent in JS).
- Allow to expose the native class constructor(s) (overloads).
- Allow to receive, in case of need, the raw V8 function callback info.
- Allow to return, in case of need, a raw V8 handle.
- Simply convert C++ data types to V8 handles and vise-versa.
- Simply access user-declared JS functions and "classes".

APIs
---------

**bridge.js** contains the main API's below:

- the **Conversion API**: Provides Type-Conversion between C++ and JavaScript.
- the **Native API**: Provides interfaces used to expose native C++ functions and classes to JS.
- the **Userland API**: Interfaces used to easily access JS data (variables, functions, "classes" etc.) in C++.


Dependencies
---------

- [**v8**](https://code.google.com/p/v8/): The backend engine used by bridge.js is Google's V8 engine. bridge.js was tested by using the latest version of v8, which is 3.25.1 (16/04/2014). That's being said, the project should support earlier builds as long as they're supporing the new Isolate API (which was introduced in 2013).
- [**boost**](http://www.boost.org) version 1.37+.

### Boost dependency

The boost libraries that **bridge.js** requires are:

- **shared_ptr**.
- **detail (only is_xxx.hpp)**.
- **mpl**.
- **preprocessor**.
- **utility (only enable_if.hpp)**.

In order to not been required to include the entire boost library, you can use Boost.Bcp utility to select just the required files for **bridge.js**.

Firstly, you should build bcp.
You can read more about BCP here: [http://www.boost.org/doc/libs/1_55_0/tools/bcp/doc/html/index.html](http://www.boost.org/doc/libs/1_55_0/tools/bcp/doc/html/index.html)

Then, you should execute the following command:
```
./bcp shared_ptr detail/is_xxx.hpp mpl preprocessor utility/enable_if.hpp ./v8bridge-boost-build
```

Finally, replace the included boost directory with the generated directory and build the project.
> Note that in case you're using XCode you have to update the project directory reference.

Include bridge.js in your C++ project
---------

Since **bridge.js** is header-only library, including it is easy!
To include **bridge.js**, just clone the library and copy-paste the downloaded v8bridge directory to your project.
In addition, you need to add v8bridge to your project include path.

To include the entire **bridge.js** API, simply include "v8bridge.hpp" in your application.

Tested Build Environment
---------

Developing and testing bridge.js was made in the following environment:

- Mac OS X 10.9.
- XCode 5.1.
- gcc++11.
- Boost library 1.55.0.
- V8 engine 3.25.1.

Samples
---------

One of my favorite parts in every README! We've prepared an awesome "sample" directory that contains many samples that are super straight-forward!

- **hello_world.cpp**: The traditional "Hello World" sample. This sample demonstrates how to create a new ScriptingEngine, create a native function endpoint (for our "void hello()" function), exposing it to JS and then triggering it by evaluating some JS code.
- **access_js_variables.cpp**: This sample demonstrates how to access some JS-declared variables from different types in C++ by using the base ScriptingEngine class, which invokes the **Conversion API**.
- **functions.cpp**: This sample demonstrates in more details how to expose a C++ native function to JS. In this demo, we're binding functions that return void and native value (int). In addition, we're declaring 2 overloads (int multiply(x) and int multiply(x, y)) that will be exposed to JS.
- **functions_v8_args.cpp**: This sample demonstrates how to access the raw V8 function callback info with the binded function.
- **functions_v8_return_handle.cpp**: This sample demonstrates how to return raw V8 handle (Handle<Value>, Handle<Array> etc.) from your binded functions.
- **functions_callback.cpp**: This sample demonstrates how to interact with callbacks that was sended to C++ from JS or to JS from C++.
- **cpp_classes_basics.cpp**: This sample demonstrates the basics of classes - how one can expose **a standard C++ class**, its methods and properties, to JS.
- **cpp_classes_static.cpp**: This sample demonstrates how to expose static methods to JS.
- **cpp_classes_ctor.cpp**: This sample demonstrates how to make your class constructors (overloads) available within JS.
- **js_functions.cpp**: This sample demonstrates how to interact with functions that was declared in JS within C++.
- **js_classes.cpp**: This sample demonstrates how to interact with "classes" that was declared in JS within C++ (create "instances", invoke methods etc.).

Sample code comparison
---------
To demonstrate how easy it is to bridge between C++ and JS, lets say that we wish to bind the following function:
```
int add(int x, int y)
{
    return x + y;
}
```

So that we can be able to execute it using JS. Then, we'd like to execute the following JS code and output its result using C++ to stdout:
```
add(1, 1);
```

#### Binding by writing raw V8 code (w/o **bridge.js**):

The code bellow uses V8 API directly to perform the task:
```
void add_callback(const FunctionCallbackInfo<Value> &info);
void add(int, int);

int main(int argc, const char *argv[])
{
    std::string scriptCode = "add(1, 1);";
    Isolate *isolate = Isolate::GetCurrent();
    Persistent<Context> *content = isolate->GetCurrentContext();

    HandleScope handle_scope(isolate);
    Context::Scope context_scope(context);
    
    Handle<FunctionTemplate> addFunction = FunctionTemplate::New(isolate, add_callback);
    Isolate::GetCurrent()
            ->GetCurrentContext()
            ->Global()
            ->Set(String::NewFromUtf8(isolate, "add"), addFunction->GetFunction());

    TryCatch try_catch(isolate);
    
    Handle<Script> compiledScript = Script::Compile(String::NewFromUtf8(isolate, scriptCode.c_str()));
    if (compiledScript.IsEmpty())
    {
        assert(try_catch.HasCaught());
        String::Utf8Value error(try_catch.Exception());
        std::cerr << "Error occured while compiling the script. Description: " << *error;
        abort();
    }
    
    Local<Value> result = compiledScript->Run();
    
    if (result.IsEmpty())
    {
        assert(try_catch.HasCaught());
        String::Utf8Value error(try_catch.Exception());
        std::cerr << "Error occured while running the script. Description: " << *error;
        abort();
    }
    
    if (!result->IsInt32())
    {
        std::cerr << "The script result is not a valid int value.";
        abort();
    }
    
    std::cout << "Result: " << result->Int32Value();
    return 0;
}

void add_callback(const FunctionCallbackInfo<Value> &info)
{
    if (info.Length() != 2)
    {
        info.GetIsolate()->ThrowException(
            String::NewFromUtf8(info.GetIsolate(), "The add function required 2 arguments.")
        );
        return;
    }
    
    if (!info[0]->IsInt32())
    {
        info.GetIsolate()->ThrowException(
            String::NewFromUtf8(info.GetIsolate(), "The x argument must be an integer.")
        );
        return;
    }
    
    if (!info[1]->IsInt32())
    {
        info.GetIsolate()->ThrowException(
            String::NewFromUtf8(info.GetIsolate(), "The y argument must be an integer.")
        );
        return;
    }
    
    int result = add(info[0]->Int32Value(), info[1]->Int32Value());
    
    info.GetReturnValue().Set(Int32::New(Isolate::GetCurrent()), result);
}
```

#### Binding by using **bridge.js**:
Now, lets perform the same task **using bridge.js**:

```
#include <v8bridge/v8bridge.hpp>

int add(int, int);

int main(int argc, const char *argv[])
{
    std::string scriptCode = "add(1, 1);";
    ScriptingEngine *engine = new ScriptingEngine();
    
    NativeFunction *addFunction = new NativeFunction(engine->getActiveIsolationScope());
    addFunction->addOverload(add); // declared above in this document
    engine->exposeFunction(addFunction, "add");
    
    std::cout << "Result: " << engine->eval<int>(scriptCode);

    delete addFunction;
    delete engine;
    return 0;
}
```

License
---------

The code base contains code of several licenses:

- **v8 engine**: V8 engine is been distributed under the BSD license.
- **boost libraries**: Boost libraries is been distributed under the Boost License.
- **bridge.js**: just like V8 engine, bridge.js is is been distributed under the BSD license.


Known Issues / TODO
---------

- The Userland API is not been registered within the Conversion API (so there's no way, for example, to get from ScriptingEngine::eval<TType> an UserlandFunction and one should request Handle<Function> and create the UserlandFunction himself/herself.
- There's no way right now to specify optional arguments for a function. This is the result of the lack of information about optional args in C++ signatures.
For example, the following functions:
```
int add(int a, int b);
int multiply(int a, int b = 2);
```

will have the same signature, a.k.a int (int, int), although multiply specify "b" as optional argument.
- A plugins/addons/extensions API will be implemented soon.
