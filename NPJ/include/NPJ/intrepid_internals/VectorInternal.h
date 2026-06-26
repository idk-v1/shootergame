// File: VectorInternal.h
///       INTREPID
///       ________
///          ||
///          ||
///         /__\
///       ,;____;,
/// ',  ,;'      ';,  ,'
///   ';'          ';'
///     ',        ,'
///
///       PROJECTS

#ifndef INTREPID_PROJECTS_INTERNAL_VECTOR_H
#define INTREPID_PROJECTS_INTERNAL_VECTOR_H

#include <ostream>
#include <type_traits>
#include "../BaseInjection.h"

#define VEC_TMPL template <typename T, size_t S, npj::VectorSemantic _NPJSem_>
#define OVEC_TMPL template <size_t Os, npj::VectorSemantic O_NPJSem_>
#define NVEC_TMPL template <typename Ot, size_t Os, npj::VectorSemantic O_NPJSem_>
#define VEC_TYPE npj::Vector<T,S,_NPJSem_>
#define OVEC_T npj::Vector<T,Os,O_NPJSem_>
#define NVEC_T npj::Vector<Ot,Os,O_NPJSem_>


#define NPJ_BINJ_BEGIN(KEY, INDEX, BASE, MEMBER, TMPL_NAMES, ...)       \
    NPJ_BINJ_STRUCT_OPEN(NPJ_MAKE_BINJ_NAME(KEY, INDEX), BASE, __VA_ARGS__) \
    T MEMBER;                                                           \
    constexpr NPJ_MAKE_BINJ_NAME(KEY, INDEX)() = default;               \
    constexpr       T& get(size_t i) {return MEMBER;}                         \
    constexpr const T& get(size_t i) const {return MEMBER;}                    \
    constexpr T& head() {return MEMBER;}                                \
    constexpr const T& head() const {return MEMBER;}                    \
    T* array() {return reinterpret_cast<T*>(this);}                     \
    NPJ_BINJ_STRUCT_CLOSE                                               \
    NPJ_MAKE_TEMPLATE(__VA_ARGS__, typename Next)                       \
        using NPJ_MAKE_BINJ_NAME_COND(KEY, INDEX) = std::conditional_t  \
        <(S == INDEX + 1), NPJ_MAKE_BINJ_NAME(KEY, INDEX)<TMPL_NAMES>, Next>;

#define NPJ_BINJ_INTER(KEY, INDEX, PREV_INDEX, MEMBER, TMPL_NAMES, ...)      \
    NPJ_BINJ_STRUCT_OPEN(NPJ_MAKE_BINJ_NAME(KEY, INDEX), NPJ_BINJ_DEF_BASE(NPJ_MAKE_BINJ_NAME(KEY, PREV_INDEX), <TMPL_NAMES>), __VA_ARGS__) \
    T MEMBER;                                                           \
    constexpr NPJ_MAKE_BINJ_NAME(KEY, INDEX)() = default;\
    constexpr       T& get(size_t i)       {return i == INDEX ? MEMBER : this->NPJ_MAKE_BINJ_NAME(KEY, PREV_INDEX)<T,S>::get(i);}\
    constexpr const T&  get(size_t i) const {return i == INDEX ? MEMBER : this->NPJ_MAKE_BINJ_NAME(KEY, PREV_INDEX)<T,S>::get(i);}\
    NPJ_BINJ_STRUCT_CLOSE                                               \
    NPJ_MAKE_TEMPLATE(__VA_ARGS__, typename Next)                       \
        using NPJ_MAKE_BINJ_NAME_COND(KEY, INDEX) = NPJ_MAKE_BINJ_NAME_COND(KEY, PREV_INDEX)<TMPL_NAMES, std::conditional_t \
        <(S == INDEX + 1), NPJ_MAKE_BINJ_NAME(KEY, INDEX)<TMPL_NAMES>, Next>>;

#define NPJ_BINJ_END(KEY, INDEX, PREV_INDEX, TMPL_NAMES, ...)                \
    NPJ_BINJ_STRUCT_OPEN(NPJ_MAKE_BINJ_NAME(KEY, INDEX), NPJ_BINJ_DEF_BASE(NPJ_MAKE_BINJ_NAME(KEY, PREV_INDEX), <TMPL_NAMES>), __VA_ARGS__) \
    T xs[S - INDEX];\
    constexpr NPJ_MAKE_BINJ_NAME(KEY, INDEX)() = default;               \
    constexpr       T& get(size_t i)      {return i < INDEX ? this->NPJ_MAKE_BINJ_NAME(KEY, PREV_INDEX)<T,S>::get(i) : xs[i - INDEX];}    \
    constexpr const T& get(size_t i) const {return i < INDEX ? this->NPJ_MAKE_BINJ_NAME(KEY, PREV_INDEX)<T,S>::get(i) : xs[i - INDEX];}    \
    NPJ_BINJ_STRUCT_CLOSE                                               \
    NPJ_MAKE_TEMPLATE(__VA_ARGS__)                                      \
        using NPJ_MAKE_BINJ_NAME_END_COND(KEY) = NPJ_MAKE_BINJ_NAME_COND(KEY, PREV_INDEX)<TMPL_NAMES, std::conditional_t \
        <(INDEX <= S), NPJ_MAKE_BINJ_NAME(KEY, INDEX)<TMPL_NAMES>, void>>;


#define NPJ_BINJ_RAW(KEY, TMPL_NAMES, ...)      \
    NPJ_BINJ_STRUCT_OPEN(NPJ_MAKE_BINJ_NAME(KEY, 0), , __VA_ARGS__)      \
    T xs[S];                                                            \
    constexpr NPJ_MAKE_BINJ_NAME(KEY, 0)() = default;                   \
    constexpr       T& get(size_t i) {return xs[i];}                          \
    constexpr const T& get(size_t i) const {return xs[i];}                     \
    constexpr T& head() {return xs[0];}                                 \
    constexpr const T& head() const {return xs[0];}                     \
    T* array() {return reinterpret_cast<T*>(this);}                     \
    NPJ_BINJ_STRUCT_CLOSE                                               \
    NPJ_MAKE_TEMPLATE(__VA_ARGS__)                                      \
        using NPJ_MAKE_BINJ_NAME_END_COND(KEY) = std::conditional_t<(0 <= S), NPJ_MAKE_BINJ_NAME(KEY, 0)<TMPL_NAMES>, void>;


#define NPJ_INIT_CHOICE(KEY, SMTC, ...)                                      \
    template<__VA_ARGS__, npj::SMTC _NPJSem_, typename Next>            \
    using NPJ_MAKE_BINJ_NAME_CHOICE(KEY, 0) = std::conditional_t<false, empty_base, Next>;
#define NPJ_DEF_CHOICE(KEY, INDEX, PREV_INDEX, COND, SMTC, TMPL_NAMES, ...) \
    template<__VA_ARGS__, npj::SMTC _NPJSem_, typename Next>            \
    using NPJ_MAKE_BINJ_NAME_CHOICE(KEY, INDEX) = NPJ_MAKE_BINJ_NAME_CHOICE(KEY, PREV_INDEX)\
        <TMPL_NAMES, _NPJSem_, std::conditional_t<_NPJSem_ == npj::SMTC::COND, NPJ_MAKE_BINJ_NAME_END_COND(COND)<TMPL_NAMES>, Next>>;
#define NPJ_END_CHOICE(KEY, PREV_INDEX, COND, SMTC, TMPL_NAMES, ...)    \
    template<__VA_ARGS__, npj::SMTC _NPJSem_>                           \
    using NPJ_MAKE_BINJ_NAME_CHOICE_END(KEY) = NPJ_MAKE_BINJ_NAME_CHOICE(KEY, PREV_INDEX)\
        <TMPL_NAMES, _NPJSem_, std::conditional_t<_NPJSem_ == npj::SMTC::COND, NPJ_MAKE_BINJ_NAME_END_COND(COND)<TMPL_NAMES>, void>>;


namespace npj
{
    enum class VectorSemantic
    {
        V_XYZW,
        XYZW,
        RGBA,
        HSVA,
        HSLA,
        UV,
        RAW
    };
}
// #define B_INJ_SYM(key, member) _
namespace npj_internal
{
struct empty_base {};

NPJ_BINJ_RAW(RAW, T COMMA S, typename T, size_t S)

NPJ_BINJ_BEGIN(XYZW, 0,  , x, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(XYZW, 1, 0, y, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(XYZW, 2, 1, z, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(XYZW, 3, 2, w, T COMMA S, typename T, size_t S)
NPJ_BINJ_END  (XYZW, 4, 3   , T COMMA S, typename T, size_t S)
//*

NPJ_BINJ_BEGIN(V_XYZW, 0,  , vx, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(V_XYZW, 1, 0, vy, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(V_XYZW, 2, 1, vz, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(V_XYZW, 3, 2, vw, T COMMA S, typename T, size_t S)
NPJ_BINJ_END  (V_XYZW, 4, 3    , T COMMA S, typename T, size_t S)

NPJ_BINJ_BEGIN(RGBA, 0,  , r, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(RGBA, 1, 0, g, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(RGBA, 2, 1, b, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(RGBA, 3, 2, a, T COMMA S, typename T, size_t S)
NPJ_BINJ_END  (RGBA, 4, 3   , T COMMA S, typename T, size_t S)

NPJ_BINJ_BEGIN(HSVA, 0,  , h, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(HSVA, 1, 0, s, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(HSVA, 2, 1, v, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(HSVA, 3, 2, a, T COMMA S, typename T, size_t S)
NPJ_BINJ_END  (HSVA, 4, 3   , T COMMA S, typename T, size_t S)

NPJ_BINJ_BEGIN(HSLA, 0,  , h, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(HSLA, 1, 0, s, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(HSLA, 2, 1, l, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(HSLA, 3, 2, a, T COMMA S, typename T, size_t S)
NPJ_BINJ_END  (HSLA, 4, 3   , T COMMA S, typename T, size_t S)

NPJ_BINJ_BEGIN(UV, 0,  , u, T COMMA S, typename T, size_t S)
NPJ_BINJ_INTER(UV, 1, 0, v, T COMMA S, typename T, size_t S)
NPJ_BINJ_END  (UV, 2, 1   , T COMMA S, typename T, size_t S)
//*/
//*
NPJ_INIT_CHOICE(VEC, VectorSemantic, typename T, size_t S)
NPJ_DEF_CHOICE(VEC, 1, 0, RAW   , VectorSemantic, T COMMA S, typename T, size_t S)
NPJ_DEF_CHOICE(VEC, 2, 1, XYZW  , VectorSemantic, T COMMA S, typename T, size_t S)
NPJ_DEF_CHOICE(VEC, 3, 2, V_XYZW, VectorSemantic, T COMMA S, typename T, size_t S)
NPJ_DEF_CHOICE(VEC, 4, 3, HSVA  , VectorSemantic, T COMMA S, typename T, size_t S)
NPJ_DEF_CHOICE(VEC, 5, 4, HSLA  , VectorSemantic, T COMMA S, typename T, size_t S)
NPJ_DEF_CHOICE(VEC, 6, 5, UV    , VectorSemantic, T COMMA S, typename T, size_t S)
NPJ_END_CHOICE(VEC,    6, RGBA  , VectorSemantic, T COMMA S, typename T, size_t S)

}



// Saves time, all is same same
#define VEC_VEC_OP(op, cnspt) \
VEC_TMPL VEC_TYPE VEC_TYPE::operator op(const VEC_TYPE& other) const \
{ \
    using Vec = VEC_TYPE;  \
    if constexpr ( cnspt<T>) \
    { \
        Vec ret;            \
        for (size_t i = 0; i < S; ++i) \
            ret[i] = (*this)[i] op other[i]; \
        return ret; \
    } \
    else \
        static_assert(always_false_v<T>, "Vector::operator"#op"(Vector&) requires that T supports the '"#op"' operator."); \
}

#define VEC_VEC_ASN_OP(op, cnspt) \
VEC_TMPL VEC_TYPE& VEC_TYPE::operator op(const VEC_TYPE& other)    \
{ \
    if constexpr ( cnspt<T>) \
    { \
        for (size_t i = 0; i < S; ++i) \
            (*this)[i] op other[i];   \
        return *this; \
    } \
    else \
        static_assert(always_false_v<T>, "Vector::operator"#op"(Vector&) requires that T supports the '"#op"' operator."); \
}

#define VEC_SCA_OP(op, cnspt) \
VEC_TMPL VEC_TYPE VEC_TYPE::operator op(const T& scalar) const    \
{ \
    using Vec = VEC_TYPE; \
    if constexpr ( cnspt<T>) \
    { \
        Vec ret; \
        for (size_t i = 0; i < S; ++i) \
            ret[i] = (*this)[i] op scalar;   \
        return ret; \
    } \
    else \
        static_assert(always_false_v<T>, "Vector::operator"#op"(Vector&) requires that T supports the '"#op"' operator."); \
}

#define VEC_SCA_ASN_OP(op, cnspt) \
VEC_TMPL VEC_TYPE& VEC_TYPE::operator op(const T& scalar)    \
{ \
    if constexpr ( cnspt<T>) \
    { \
        for (size_t i = 0; i < S; ++i) \
            (*this)[i] op scalar; \
        return *this; \
    } \
    else \
        static_assert(always_false_v<T>, "Vector::operator"#op"(Vector&) requires that T supports the '"#op"' operator."); \
}

#undef VEC_TEMPL
#endif
