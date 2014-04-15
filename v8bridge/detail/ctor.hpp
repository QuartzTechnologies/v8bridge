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

/**
 * Since we can't receive ctor signature, we shall use a typed_list that will hold the ctor arg types.
 */

#ifndef v8bridge_ctor_hpp
#   define v8bridge_ctor_hpp

#   include <v8bridge/detail/prefix.hpp>
#   include <v8bridge/detail/preprocessor.hpp>
#   include <v8bridge/detail/type_list.hpp>

#include <v8bridge/detail/typeid.hpp>

#   include <boost/mpl/if.hpp>
#   include <boost/mpl/eval_if.hpp>
#   include <boost/mpl/size.hpp>
#   include <boost/mpl/iterator_range.hpp>
#   include <boost/mpl/empty.hpp>
#   include <boost/mpl/begin_end.hpp>
#   include <boost/mpl/bool.hpp>
#   include <boost/mpl/prior.hpp>
#   include <boost/mpl/joint_view.hpp>
#   include <boost/mpl/back.hpp>

#   include <boost/type_traits/is_same.hpp>

#   include <boost/preprocessor/enum_params_with_a_default.hpp>
#   include <boost/preprocessor/enum_params.hpp>

#   define V8_BRIDGE_OVERLOAD_TYPES_WITH_DEFAULT_VALUE  BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(V8_MAX_ARITY, class TClass, mpl::void_)

#   define V8_BRIDGE_OVERLOAD_TYPES                     BOOST_PP_ENUM_PARAMS_Z(1, V8_MAX_ARITY, class TClass)

#   define V8_BRIDGE_OVERLOAD_ARGS                     BOOST_PP_ENUM_PARAMS_Z(1, V8_MAX_ARITY, TClass)

namespace v8
{
    namespace bridge
    {
        using namespace boost;
        
        template <V8_BRIDGE_OVERLOAD_TYPES_WITH_DEFAULT_VALUE>
        class ctor; // forward declaration
        
        
        template <V8_BRIDGE_OVERLOAD_TYPES_WITH_DEFAULT_VALUE>
        struct optional; // forward declaration
        
        namespace detail
        {
            template <class TClass>
            struct is_optional
            : mpl::false_
            {};
            
            template <V8_BRIDGE_OVERLOAD_TYPES>
            struct is_optional<optional<V8_BRIDGE_OVERLOAD_ARGS> >
            : mpl::true_
            {};
            
            template <class S>
            struct step_backward : mpl::iterator_range<
                typename mpl::begin<S>::type,
                typename mpl::prior<
                    typename mpl::end<S>::type
                >::type
            > {};
            
            template <int NDefaults>
            struct define_ctor_helper;
        }
        
        template <class DerivedT>
        struct ctor_base
        {
            template <class classT>
            void define(classT& cl) const
            {
                typedef typename DerivedT::signature signature;
                typedef typename DerivedT::n_arguments n_arguments;
                typedef typename DerivedT::n_defaults n_defaults;
                
                
                ::v8::bridge::detail::define_ctor_helper<n_defaults::value>::apply(
                                                                           cl
                                                                           , signature()
                                                                           , n_arguments());
            }
        };
        
        template <V8_BRIDGE_OVERLOAD_TYPES>
        class ctor : public ctor_base<ctor<V8_BRIDGE_OVERLOAD_ARGS> >
        {
        private:
            typedef ctor_base<ctor<V8_BRIDGE_OVERLOAD_ARGS> > base;
        public:
            ctor() { }
            
            
            typedef detail::type_list<V8_BRIDGE_OVERLOAD_ARGS> signature_;
            
            typedef detail::is_optional<
            typename mpl::eval_if<
            mpl::empty<signature_>
            , mpl::false_
            , mpl::back<signature_>
            >::type
            > back_is_optional;
            
            typedef typename mpl::eval_if<
            back_is_optional
            , mpl::back<signature_>
            , mpl::vector0<>
            >::type optional_args;
            
            typedef typename mpl::eval_if<
                back_is_optional
                , mpl::if_<
            mpl::empty<optional_args>
            , ::v8::bridge::detail::step_backward<signature_>
            , mpl::joint_view<
            ::v8::bridge::detail::step_backward<signature_>
            , optional_args
            >
            >
            , signature_
            >::type signature;
            
            // TODO: static assert to make sure there are no other optional elements
            
            // Count the number of default args
            typedef mpl::size<optional_args> n_defaults;
            typedef mpl::size<signature> n_arguments;
        };
        
        template <V8_BRIDGE_OVERLOAD_TYPES>
        struct optional : ::v8::bridge::detail::type_list<V8_BRIDGE_OVERLOAD_ARGS>
        {
        };
        
        namespace detail
        {
            
            //  define_class_init_helper<N>::apply
            //
            //      General case
            //
            //      Accepts a class_ and an arguments list. Defines a constructor
            //      for the class given the arguments and recursively calls
            //      define_class_init_helper<N-1>::apply with one fewer argument (the
            //      rightmost argument is shaved off)
            template <int NDefaults>
            struct define_ctor_helper
            {
                template <class ClassT, class Signature, class NArgs>
                inline static void apply(
                                  ClassT& cl
                                  , Signature const& args
                                  , NArgs)
                {
                    cl.template exposeCtor<Signature, NArgs>();
                    
                    typedef typename mpl::prior<NArgs>::type next_nargs;
                    define_ctor_helper<NDefaults - 1>::apply(cl, Signature(), next_nargs());
                }
            };
            
            //  define_class_init_helper<0>::apply
            //
            //      Terminal case
            //
            //      Accepts a class_ and an arguments list. Defines a constructor
            //      for the class given the arguments.
            template<>
            struct define_ctor_helper<0>
            {
                template <class ClassT, class Signature, class NArgs>
                inline static void apply(
                                  ClassT& cl
                                  , Signature const& args
                                  , NArgs)
                {
                    cl.template exposeCtor<Signature, NArgs>();
                }
            };
        }
    }
}

#   undef V8_BRIDGE_OVERLOAD_TYPES_WITH_DEFAULT_VALUE
#   undef V8_BRIDGE_OVERLOAD_TYPES
#   undef V8_BRIDGE_OVERLOAD_ARGS

#endif
