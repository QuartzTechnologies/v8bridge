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

#ifndef v8bridge_utilities_hpp
#define v8bridge_utilities_hpp

#include <v8bridge/detail/prefix.hpp>

#include <string>

namespace v8
{
    namespace bridge
    {
        /**
         * Escape the given string
         *
         * Modified version of the s11n object serialization/persistence library.
         */
        inline size_t escape(std::string value, const std::string &to_esc, const std::string & esc)
        {
            std::string::size_type pos;
            pos = value.find_first_of( to_esc );
            size_t reps = 0;
            
            while( pos != std::string::npos )
            {
                value.insert( pos, esc );
                ++reps;
                pos = value.find_first_of( to_esc, pos + esc.size() + 1 );
            }
            
            return reps;
        }
        
        inline V8_DECL std::string escape_js_value(std::string value)
        {
            std::string output(value);
            escape(output, "\'", "\\");
            return output;
        }
        
        inline V8_DECL std::string escape_js_quotes(std::string value)
        {
            if(std::string::npos == value.find('\''))
            {
                return "'" + value + "'";
            }
            else if (std::string::npos == value.find("\""))
            {
                return "\"" + value + "\"";
            }
            else
            {
                std::string output(value);
                escape(output, "\'", "\\");
                return "\"" + output + "\"";
            }
        }
    }
}

#endif
