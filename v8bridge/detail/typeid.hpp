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

#ifndef V8Bridge_typeid_h
#define V8Bridge_typeid_h

namespace v8
{
    namespace bridge
    {
        /*
         * This file is been used to provide simple interface for TypeId representation.
         *
         * We got 2 compiler-specific implementations:
         *  - GCC specific, which uses __PRETTY_FUNCTION__ and does not need typeid (which means that there's no need to gcc++11)
         *  - MSVC specific, which uses __FUNCTION__.
         *  - general fallback, which does use typeid.
         *
         * As a fallback, it's using the typeid() operator - "And May the Odds be in Your Favor".
         *
         * For more info, See: http://stackoverflow.com/questions/1666802/is-there-a-class-macro-in-c
         */
        
        //-------------------------------------------------
        //  GNU specific
        //-------------------------------------------------
#if defined(__GNUC__)
        template <class TClass>
        class TypeId
        {
        public:
            TypeId() { }
            
            bool operator!=(TypeId const& other) const
            {
                return this->name() != other->name();
            }
            
            bool operator==(TypeId const& other) const
            {
                return this->name() == other->name();
            }
            
            inline std::string name() const
            {
                return this->getClassName(__PRETTY_FUNCTION__);
            }
            
        private:
            inline std::string getClassName(std::string prettyFunction) const
            {
                /*
                 * Sample input: std::string v8::bridge::TypeId<Foo<Car> >::name() const [TClass = Foo<Car>]Foo<Car>
                 */
                
                //-------------------------------------------------
                //  Search for the "[TClass = " prefix
                //-------------------------------------------------
                
                size_t bracketsPrefix = prettyFunction.find("[TClass = ");
                if (bracketsPrefix == std::string::npos)
                {
                    return "<Unknown>";
                }
                
                bracketsPrefix += 10; //strlen("[TClass = ");
                
                //-------------------------------------------------
                //  Search for the brackets suffix
                //-------------------------------------------------
                size_t bracketsSufix = prettyFunction.find("]", bracketsPrefix - 1);
                if (bracketsSufix == std::string::npos)
                {
                    return "<Unknown>";
                }
                
                //-------------------------------------------------
                //  Cut
                //-------------------------------------------------
                return prettyFunction.substr(bracketsPrefix, bracketsSufix - bracketsPrefix);
            }
        };
#elif WIN32
#	pragma warning(push)
#	include <iostream>
        
		template <class TClass>
		class TypeId
		{
		public:
			TypeId() { }
            
			bool operator!=(TypeId const& other) const
			{
				return this->name() != other->name();
			}
            
			bool operator==(TypeId const& other) const
			{
				return this->name() == other->name();
			}
            
			inline std::string name() const
			{
				return this->getClassName(__FUNCTION__);
			}
            
		private:
			inline std::string getClassName(std::string functionDescription) const
			{
				/*
                 * Sample input: v8::bridge::TypeId<class Foo<class Car> >::name
                 */
                
				//-------------------------------------------------
				//  Search for the "TypeId<" prefix
				//-------------------------------------------------
                
				size_t bracketsPrefix = functionDescription.find("TypeId<");
				if (bracketsPrefix == string::npos)
				{
					return "<Unknown>";
				}
                
				bracketsPrefix += 7; // strlen("TypeId<")
                
				//-------------------------------------------------
				//	Find the colons, which marks us a suffix
				//-------------------------------------------------
                
				size_t bracketsSuffix = functionDescription.find("::", bracketsPrefix);
				if (bracketsSuffix == string::npos)
				{
					return "<Unknown>";
				}
                
				functionDescription = functionDescription.substr(bracketsPrefix, bracketsSuffix - bracketsPrefix);
                
				//-------------------------------------------------
				//	Remove the extra "class" keyword
				//
				//	Example: "class Foo<class Car> >"
				//-------------------------------------------------
                
				size_t classKeywordPos = string::npos;
				while ( (classKeywordPos = functionDescription.find("class ")) != string::npos )
				{
					functionDescription = functionDescription.substr(0, classKeywordPos) + functionDescription.substr(classKeywordPos + 6 /* = strlen("class ") */);
				}
                
				return functionDescription;
			}
		};
        
#	pragma warning(pop)
#else
#   include <typeinfo>
        
        template <class TClass>
        class TypeId
        {
        public:
            TypeId() : id(&typeid(TClass))
            {}
            
            bool operator!=(TypeId const& other) const
            {
                return *id != *other.id;
            }
            
            bool operator==(TypeId const& other) const
            {
                return *id == *other.id;
            }
            
            inline char const* name() const
            {
                return id->name();
            }
            
        private:
            type_info const* id;
        };
#endif
    }
}

#endif
