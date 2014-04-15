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
 * This file is based on boost python binding by David Abrahams (2002)
 * that was distributed under the Boost Software License, Version 1.0.
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef v8bridge_forward_hpp
#define v8bridge_forward_hpp

#include <v8bridge/detail/prefix.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_scalar.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/ref.hpp>
#include <v8bridge/detail/is_auto_ptr.hpp>
#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
#   include <boost/type_traits/is_enum.hpp>
#   include <boost/mpl/and.hpp>
#   include <boost/mpl/not.hpp>
#else
#   include <boost/mpl/or.hpp>
#endif

namespace v8
{
    namespace bridge
    {
        namespace detail
        {
            // Very much like boost::reference_wrapper<T>, except that in this
            // case T can be a reference already without causing a
            // reference-to-reference error.
            template <class T>
            struct reference_to_value
            {
                typedef typename add_reference<typename add_const<T>::type>::type reference;
                
                reference_to_value(reference x) : m_value(x) {}
                reference get() const { return m_value; }
            private:
                reference m_value;
            };
            
            template <class TType>
            struct copy_ctor_mutates
            : is_auto_ptr<TType>
            {
            };
            
            template <class T>
            struct value_arg : mpl::if_<
            copy_ctor_mutates<T>
            , T
            , typename add_reference<
                typename add_const<T>::type
            >::type > {};
            
            // A little metaprogram which selects the type to pass through an
            // intermediate forwarding function when the destination argument type
            // is T.
            template <class T>
            struct forward : mpl::if_<
# if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
            // vc6 chokes on unforwarding enums nested in classes
            mpl::and_<is_scalar<T> , mpl::not_<is_enum<T> >
            >
# else
            mpl::or_<copy_ctor_mutates<T>, is_scalar<T> >
# endif
            , T
            , reference_to_value<T> >
            {
            };
            
            template<typename T>
            struct unforward
            {
                typedef typename unwrap_reference<T>::type& type;
            };
            
            template<typename T>
            struct unforward<reference_to_value<T> >
            {
                typedef T type;
            };
            
            template <typename T>
            struct unforward_cref : value_arg<
                typename unwrap_reference<T>::type
            > { };
            
            template<typename T>
            struct unforward_cref<reference_to_value<T> >
            : add_reference<typename add_const<T>::type> { };
        }
    }
}

#endif
