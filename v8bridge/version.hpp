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


#ifndef v8bridge_version_hpp
#   define v8bridge_version_hpp
#   include <v8bridge/detail/prefix.hpp>

#   define V8BRIDGE_MAJOR_VERSION                   0
#   define V8BRIDGE_MINOR_VERSION                   1
#   define V8BRIDGE_BUILD_VERSION                   0

#   define V8BRIDGE_VERSION_IS_RC                   0
#   define V8BRIDGE_VERSION_IS_STABLE               0

#   ifndef V8BRIDGE_VERSION_TAG
#       if V8BRIDGE_VERSION_IS_STABLE
#           define V8BRIDGE_VERSION_TAG             ""
#       elif V8BRIDGE_VERSION_IS_RC
#           define V8BRIDGE_VERSION_TAG             " (candidate)"
#       else
#           define V8BRIDGE_VERSION_TAG             "-pre"
#       endif
#   endif

#   ifndef V8BRIDGE_VERSION_STRING
#       define V8BRIDGE_VERSION_STRING_DUMP_EX(t)   #t
#       define V8BRIDGE_VERSION_STRING_DUMP(t)      V8BRIDGE_VERSION_STRING_DUMP_EX(t)
#   endif

#   define V8BRIDGE_VERSION_STRING                  V8BRIDGE_VERSION_STRING_DUMP(V8BRIDGE_MAJOR_VERSION) "." \
                                                    V8BRIDGE_VERSION_STRING_DUMP(V8BRIDGE_MINOR_VERSION) "." \
                                                    V8BRIDGE_VERSION_STRING_DUMP(V8BRIDGE_BUILD_VERSION) \
                                                    V8BRIDGE_VERSION_TAG

#   define V8BRIDGE_VERSION_AT_LEAST(major, minor, build) \
                                                    (( (major) < V8BRIDGE_MAJOR_VERSION) \
                                                    || ((major) == V8BRIDGE_MAJOR_VERSION && (minor) < V8BRIDGE_MINOR_VERSION) \
                                                    || ((major) == V8BRIDGE_MAJOR_VERSION && \
                                                    (minor) == V8BRIDGE_MINOR_VERSION && (build) <= V8BRIDGE_BUILD_VERSION))

#endif
