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
 *  This file generates caller structure that allows to invoke a given function with V8 passed args.
 *
 *  This file generates caller_arity specific implementation for each number of arguments arity.
 *
 *  The following macros generate expansions for the following caller_arity's::
 *
 *      template <>
 *      struct caller_arity<N>
 *      {
 *          struct impl
 *          {
 *          public:
 *              impl(Isolate *isolate, TPointer p);
 *
 *              // Gets the number arity for the given specific implementation
 *              static unsigned getArity() { return N; }
 *
 *              // Invoke the fucntion/method and get the result as V8 argument (converted by NativeToJs)
 *              // or v8::Undefined if this is a void function/method
 *              Handle<Value> operator () (const FunctionCallbackInfo<Value> &info);
 *
 *              // Invoke the given function/method get the result as C++ native type
 *              Handle<Value> invoke (const FunctionCallbackInfo<Value> &info);
 *
 *              // Check if the given call info (args etc.) can be used to invoke the given function (type check etc.)
 *              bool isInvokable (const FunctionCallbackInfo<Value> &info)
 *          }
 *      }
 *
 */


#if !defined(BOOST_PP_IS_ITERATING)
#   ifndef v8bridge_native_caller_hpp
#       define v8bridge_native_caller_hpp
#       include <v8bridge/detail/prefix.hpp>
#       include <v8bridge/detail/typeid.hpp>
#       include <v8bridge/detail/preprocessor.hpp>
#       include <v8bridge/conversion.hpp>
#       include <v8bridge/native/invoke_native.hpp>

#       include <boost/preprocessor/iterate.hpp>
#       include <boost/preprocessor/cat.hpp>
#       include <boost/preprocessor/dec.hpp>
#       include <boost/preprocessor/if.hpp>
#       include <boost/preprocessor/iteration/local.hpp>
#       include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#       include <boost/preprocessor/repetition/repeat.hpp>

#       include <boost/type_traits/is_same.hpp>
#       include <boost/type_traits/is_convertible.hpp>

#       include <boost/mpl/apply.hpp>
#       include <boost/mpl/eval_if.hpp>
#       include <boost/mpl/identity.hpp>
#       include <boost/mpl/size.hpp>
#       include <boost/mpl/at.hpp>
#       include <boost/mpl/int.hpp>
#       include <boost/mpl/next.hpp>
#       include <boost/mpl/joint_view.hpp>
#       include <boost/mpl/pop_front.hpp>
#       include <boost/utility/enable_if.hpp>
#       include <boost/type_traits/remove_const.hpp>
#       include <boost/type_traits/remove_reference.hpp>

namespace v8
{
    namespace bridge
    {
        using namespace v8;
        using namespace boost;
        
        template <unsigned> struct caller_arity;
        
        template <class TPointer, class TSignature>
        struct caller;
        
#       define V8_BRIDGE_ARG_NEXT(init, name, n)     typedef BOOST_PP_IF(n, typename mpl::next<BOOST_PP_CAT(name, BOOST_PP_DEC(n)) >::type, init) name##n;
#       define V8_BRIDGE_SETUP_ARG(n, tfirst)                                         \
        V8_BRIDGE_ARG_NEXT(typename mpl::next<tfirst>::type, TArgType, n) \
        Handle<Value> arg##n = info[mpl::int_<n>()]; \
        BOOST_DEDUCED_TYPENAME TArgType##n::type resolvedArg##n; \
        JsToNative(this->m_isolationScope, resolvedArg##n, arg##n);
        
#       define V8_BRIDGE_ARG_IS_CONVERTABLE(n, tfirst)                                         \
        V8_BRIDGE_ARG_NEXT(typename mpl::next<tfirst>::type, TArgType, n) \
        Handle<Value> arg##n = info[mpl::int_<n>()]; \
        if (!IsJsToNativeConvertable<BOOST_DEDUCED_TYPENAME TArgType##n::type>(this->m_isolationScope, arg##n)) { \
            return false; \
        }
        
#       define BOOST_PP_ITERATION_PARAMS_1           (3, (0, V8_MAX_ARITY + 1, <v8bridge/native/native_caller.hpp>))
#       include BOOST_PP_ITERATE()
        
#       undef V8_BRIDGE_ARG_NEXT
        
        template <class TPointer, class TSignature>
        struct caller_base_select
        {
            enum { arity = boost::is_member_function_pointer<TPointer>::value ? mpl::size<TSignature>::value - 2 : mpl::size<TSignature>::value - 1 };
            typedef typename caller_arity<arity>::template impl<TPointer, TSignature> type;
        };
        
        template <class TPointer, class TSignature>
        struct caller : caller_base_select<TPointer, TSignature>::type
        {
            typedef typename caller_base_select<TPointer, TSignature>::type base;
            typedef Handle<Value> result_type;
            
            caller(Isolate *isolate, TPointer pointer) : base(isolate, pointer) { };
        };
    }
}
#   endif // v8bridge_caller_hpp
#else // BOOST_PP_IS_ITERATING
#   define N BOOST_PP_ITERATION()

template <>
struct caller_arity<N>
{
    template <class TPointer, class TSignature>
    struct impl
    {
    public:
        impl(Isolate *isolate, TPointer p) : m_callbackPointer(p), m_isolationScope(isolate) { };
        
        inline static unsigned getArity() { return N; }
        
        /* Execute in method context */
        template <class TCallback = TPointer>
        inline typename boost::enable_if<boost::is_member_function_pointer<TCallback>, Handle<Value> >::type
        operator () (const FunctionCallbackInfo<Value> &info)
        {
            typedef typename mpl::begin<TSignature>::type TSeqFirst;
            typedef typename TSeqFirst::type TResult;
            typedef typename mpl::next< TSeqFirst >::type TSeqInstanceType;
            
            typedef typename boost::remove_const<typename boost::remove_reference<typename TSeqInstanceType::type>::type >::type TInstanceType;
            
            /* Retrieve "this" from V8 */
            Local<Object> _this = info.Holder();
            
            /* Get address by casting the External saved value.
             Note that it was saved in NativeClass<TClass>::ctor */
            TInstanceType *instance = static_cast<TInstanceType *>(_this->GetAlignedPointerFromInternalField(_this->InternalFieldCount() - 2));
            
#if V8BRIDGE_DEBUG /* By default, we're always calling isInvokable before operator() (see NativeFunction), so we don't need this.
I'm leaving this validation in operator() too only for debugging sake. */
            if (!this->isInvokable(info)) {
                return v8::Undefined(this->m_isolationScope);
            }
#endif
            
# if N
#  define BOOST_PP_LOCAL_MACRO(i) V8_BRIDGE_SETUP_ARG(i, TSeqInstanceType)
#  define BOOST_PP_LOCAL_LIMITS (0, N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif
            
            Handle<Value> result = bridge::detail::invoke_native(
                                                                 this->m_isolationScope,
                                                                 detail::invoke_tag<TResult, TPointer>()
                                                                 , this->m_callbackPointer
                                                                 , instance
                                                                 BOOST_PP_ENUM_TRAILING_PARAMS(N, resolvedArg)
                                                                 );
            
            
            return result;
        }
        
        /* Execute in function context */
        template <class TCallback = TPointer>
        inline typename boost::disable_if<boost::is_member_function_pointer<TCallback>, Handle<Value> >::type
        operator () (const FunctionCallbackInfo<Value> &info)
        {
            typedef typename mpl::begin<TSignature>::type TSeqFirst;
            typedef typename TSeqFirst::type TResult;
            
#if V8BRIDGE_DEBUG /* By default, we're always calling isInvokable before operator(), so we don't need this.
I'm leaving this in operator() too only for debugging preposes. */
            if (!this->isInvokable(info)) {
                return v8::Undefined(this->m_isolationScope);
            }
#endif
            
# if N
#  define BOOST_PP_LOCAL_MACRO(i) V8_BRIDGE_SETUP_ARG(i, TSeqFirst)
#  define BOOST_PP_LOCAL_LIMITS (0, N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif
            
            
            Handle<Value> result = bridge::detail::invoke_native(
                                                                 this->m_isolationScope,
                                                                 detail::invoke_tag<TResult, TPointer>()
                                                                 , this->m_callbackPointer
                                                                 BOOST_PP_ENUM_TRAILING_PARAMS(N, resolvedArg)
                                                                 );
            
            
            return result;
        }
        
        /* Execute in method context */
        template <class TResult, class TCallback = TPointer>
        inline typename boost::enable_if<boost::is_member_function_pointer<TCallback>, TResult >::type
        invoke (const FunctionCallbackInfo<Value> &info)
        {
            typedef typename mpl::begin<TSignature>::type TSeqFirst;
            typedef typename mpl::next< TSeqFirst >::type TSeqInstanceType;
            
            typedef typename boost::remove_const<typename boost::remove_reference<typename TSeqInstanceType::type>::type >::type TInstanceType;
            
            /* Retrieve "this" from V8 */
            Local<Object> _this = info.Holder();
            
            /* Get address by casting the External saved value.
             Note that it was saved in NativeClass<TClass>::ctor */
            TInstanceType *instance = static_cast<TInstanceType *>(_this->GetAlignedPointerFromInternalField(_this->InternalFieldCount() - 2));
            
# if N
#  define BOOST_PP_LOCAL_MACRO(i) V8_BRIDGE_SETUP_ARG(i, TSeqInstanceType)
#  define BOOST_PP_LOCAL_LIMITS (0, N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif
            
            return bridge::detail::invoke_native_raw<TResult>(
                                                     this->m_isolationScope
                                                     , detail::invoke_tag<TResult, TPointer>()
                                                     , this->m_callbackPointer
                                                     , instance
                                                     BOOST_PP_ENUM_TRAILING_PARAMS(N, resolvedArg)
                                                     );
        }
        
        /* Execute in function context */
        template <class TResult, class TCallback = TPointer>
        inline typename boost::disable_if<boost::is_member_function_pointer<TCallback>, TResult >::type
        invoke (const FunctionCallbackInfo<Value> &info)
        {
            typedef typename mpl::begin<TSignature>::type TSeqFirst;
            
# if N
#  define BOOST_PP_LOCAL_MACRO(i) V8_BRIDGE_SETUP_ARG(i, TSeqFirst)
#  define BOOST_PP_LOCAL_LIMITS (0, N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif
            
            
            return bridge::detail::invoke_native_raw<TResult>(
                                                     this->m_isolationScope,
                                                     detail::invoke_tag<TResult, TPointer>()
                                                     , this->m_callbackPointer
                                                     BOOST_PP_ENUM_TRAILING_PARAMS(N, resolvedArg)
                                                     );
            
            
        }
        
        /* Checking if the function is invokable in method context */
        template <class TCallback = TPointer>
        inline typename boost::enable_if<boost::is_member_function_pointer<TCallback>, bool >::type
        isInvokable(const FunctionCallbackInfo<Value> &info)
        {
            typedef typename mpl::begin<TSignature>::type TSeqFirst;
            typedef typename TSeqFirst::type TResult;
            typedef typename mpl::next< TSeqFirst>::type TSeqInstanceType;
            
#   if N
#       define BOOST_PP_LOCAL_MACRO(i) V8_BRIDGE_ARG_IS_CONVERTABLE(i, TSeqInstanceType)
#       define BOOST_PP_LOCAL_LIMITS (0, N-1)
#       include BOOST_PP_LOCAL_ITERATE()
#   endif
            
            return true;
        }
        
        
        /* Checking if the function is invokable in function context */
        template <class TCallback = TPointer>
        inline typename boost::disable_if<boost::is_member_function_pointer<TCallback>, bool >::type
        isInvokable(const FunctionCallbackInfo<Value> &info)
        {
            typedef typename mpl::begin<TSignature>::type TSeqFirst;
            typedef typename TSeqFirst::type TResult;
            
#   if N
#       define BOOST_PP_LOCAL_MACRO(i) V8_BRIDGE_ARG_IS_CONVERTABLE(i, TSeqFirst)
#       define BOOST_PP_LOCAL_LIMITS (0, N-1)
#       include BOOST_PP_LOCAL_ITERATE()
#   endif
            
            return true;
        }
    private:
        Isolate *m_isolationScope;
        TPointer m_callbackPointer;
    };
};

#endif