// File: Vector.h
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

#ifndef INTREPID_PROJECTS_COMMON_VECTOR_H
#define INTREPID_PROJECTS_COMMON_VECTOR_H

#include <cassert>
#include <cmath>
#include <functional>
#include <ostream>
#include "./intrepid_internals/VectorInternal.h"
#include "Concepts.h"
#include <array>

namespace npj
{
    template <typename T, size_t S,
              VectorSemantic _NPJSem_ = VectorSemantic::XYZW
              >
    struct Vector : npj_internal::NPJ_MAKE_BINJ_NAME_CHOICE_END(VEC)<T,S, _NPJSem_>
    {
        using this_t = VEC_TYPE;
        using base_t = npj_internal::NPJ_MAKE_BINJ_NAME_CHOICE_END(VEC)<T,S, _NPJSem_>;
        
        constexpr Vector() = default;
        constexpr Vector(const T arr[], int N);
        
        template <typename... Args>
        constexpr Vector(Args... args);
        
        constexpr       T& operator[](const size_t i);
        constexpr const T& operator[](const size_t i) const;
        
        Vector  operator+ (const Vector &other) const;
        Vector  operator- (const Vector &other) const;
        Vector  operator* (const Vector &other) const;
        Vector  operator/ (const Vector &other) const;
        Vector  operator% (const Vector &other) const;
        Vector& operator+=(const Vector &other);
        Vector& operator-=(const Vector &other);
        Vector& operator*=(const Vector &other);
        Vector& operator/=(const Vector &other);
        Vector& operator%=(const Vector &other);
        
        Vector  operator+ (const T      &scalar) const;
        Vector  operator- (const T      &scalar) const;
        Vector  operator* (const T      &scalar) const;
        Vector  operator/ (const T      &scalar) const;
        Vector  operator% (const T      &scalar) const;
        Vector& operator+=(const T      &scalar);
        Vector& operator-=(const T      &scalar);
        Vector& operator*=(const T      &scalar);
        Vector& operator/=(const T      &scalar);
        Vector& operator%=(const T      &scalar);
        
        Vector& operator=(const Vector &other);


        template <VectorSemantic O>
        constexpr Vector<T, S, O> rename();
        constexpr T       dot(const Vector &other) const;
        constexpr T       dist(const Vector &other) const;
        constexpr T       sqdist(const Vector &other) const;
        constexpr T       magnitude() const;
        constexpr T       length() const;
        constexpr T       norm() const;
        constexpr T       squaredLength() const;
        constexpr T       lengthSquared() const;
        constexpr T       sqlen() const;
        constexpr T       minValue() const;
        constexpr T       maxValue() const;
        constexpr Vector& normalize(const T& len = -1);
        constexpr Vector  lerp(const Vector& other, T ratio) const;
        constexpr Vector  cross(const Vector& other) const requires (S == 3);
        
        static Vector  zero();
    };

    
    VEC_TMPL T dot(const VEC_TYPE &a, const VEC_TYPE &b);
    VEC_TMPL T magnitude(const VEC_TYPE &v);
    VEC_TMPL T length(const VEC_TYPE &v);
    VEC_TMPL T norm(const VEC_TYPE &v);
    VEC_TMPL T squaredLength(const VEC_TYPE &v);
    VEC_TMPL T lengthSquared(const VEC_TYPE &v);
    VEC_TMPL T sqlen(const VEC_TYPE &v);
    VEC_TMPL T minValue(const VEC_TYPE &v);
    VEC_TMPL T maxValue(const VEC_TYPE &v);
    VEC_TMPL VEC_TYPE& normalize(const VEC_TYPE &v  , const T& len = -1);
    VEC_TMPL VEC_TYPE  relative (const VEC_TYPE &a  , const VEC_TYPE &b);
    VEC_TMPL VEC_TYPE  radial   (const VEC_TYPE &vec, const VEC_TYPE &dir, const bool dirNormalized = false);
    VEC_TMPL VEC_TYPE  lateral  (const VEC_TYPE &vec, const VEC_TYPE &dir, const bool dirNormalized = false);
    VEC_TMPL VEC_TYPE  lerp     (const VEC_TYPE &a, const VEC_TYPE &b, T ratio);
    VEC_TMPL VEC_TYPE  cross    (const VEC_TYPE &a, const VEC_TYPE &v) requires (S == 3);

    //VEC_TMPL VEC_TYPE                     min(std::initializer_list<VEC_TYPE>);
    //VEC_TMPL_OPEN, typename CMP> VEC_TYPE min(std::initializer_list<VEC_TYPE>, CMP cmp = {});
    //VEC_TMPL VEC_TYPE                     min(std::initializer_list<VEC_TYPE>, std::function<bool(VEC_TYPE, VEC_TYPE)> cmp);

    //VEC_TMPL VEC_TYPE                     max(std::initializer_list<VEC_TYPE>);
    //VEC_TMPL_OPEN, typename CMP> VEC_TYPE max(std::initializer_list<VEC_TYPE>, CMP cmp = {});
    //VEC_TMPL VEC_TYPE                     max(std::initializer_list<VEC_TYPE>, std::function<bool(VEC_TYPE, VEC_TYPE)> cmp);
}

VEC_TMPL std::ostream& operator<<(std::ostream& o, const VEC_TYPE &v)
{
    o << '{';

    for (int i = 0; i < S - 1; ++i)
        o << v[i] << ", ";
    
    o << v[S-1] << '}';

    return o;
}

VEC_TMPL
constexpr VEC_TYPE::Vector(const T arr[], int N)
{
    for (size_t i = 0; i < N; ++i)
        (*this)[i] = arr[i];
}

VEC_TMPL
template <npj::VectorSemantic O>
constexpr npj::Vector<T, S, O> VEC_TYPE::rename()
{
    npj::Vector<T,S,O> ret;
    for (int i = 0; i < S; ++i)
        ret[i] = (*this)[i];
    return ret;
}

VEC_TMPL
template <typename... Args>
constexpr VEC_TYPE::Vector(Args... args) 
{
    static_assert(sizeof...(Args) <= S, "Too many arguments");
    
    auto arr = std::array<T, sizeof...(Args)>{ static_cast<T>(args)... };
    
    if constexpr (std::is_constant_evaluated && sizeof...(Args) < S)
        for(size_t i = sizeof...(Args); i < S; ++i)
            (*this)[i] = {};
    
    for (std::size_t i = 0; i < sizeof...(Args); ++i) {
        (*this)[i] = arr[i];
    }
}

VEC_TMPL
constexpr T& VEC_TYPE::operator[](const size_t index)
{
    if consteval
    { return this->base_t::get(index); }

    return *(&this->head() + index);
}

VEC_TMPL
constexpr const T& VEC_TYPE::operator[](const size_t index) const
{
    if consteval
    { return this->base_t::get(index); }

    return *(&this->head() + index);
}

VEC_VEC_OP(+, Addable)
VEC_VEC_OP(-, Subtractable)
VEC_VEC_OP(*, Multipliable)
VEC_VEC_OP(/, Dividable)
VEC_VEC_OP(%, Modable)
VEC_VEC_ASN_OP(+=, AddAssignable)
VEC_VEC_ASN_OP(-=, SubAssignable)
VEC_VEC_ASN_OP(*=, MulAssignable)
VEC_VEC_ASN_OP(/=, DivAssignable)
VEC_VEC_ASN_OP(%=, ModAssignable)

VEC_SCA_OP(+, Addable)
VEC_SCA_OP(-, Subtractable)
VEC_SCA_OP(*, Multipliable)
VEC_SCA_OP(/, Dividable)
VEC_SCA_OP(%, Modable)
VEC_SCA_ASN_OP(+=, AddAssignable)
VEC_SCA_ASN_OP(-=, SubAssignable)
VEC_SCA_ASN_OP(*=, MulAssignable)
VEC_SCA_ASN_OP(/=, DivAssignable)
VEC_SCA_ASN_OP(%=, ModAssignable)

VEC_TMPL VEC_TYPE& VEC_TYPE::operator=(const VEC_TYPE &other)
{
    for (size_t i = 0; i < S; ++i)
        (*this)[i] = other[i];

    return *this;
}
VEC_TMPL
constexpr T VEC_TYPE::dist(const VEC_TYPE &other) const { return (*this - other).length(); }

VEC_TMPL
constexpr T VEC_TYPE::sqdist(const VEC_TYPE &other) const { return (*this - other).sqlen(); }

VEC_TMPL
constexpr T VEC_TYPE::dot(const VEC_TYPE &other) const {
    T ret = { 0 };
    for (size_t i = 0; i < S; ++i)
        ret += (*this)[i] * other[i];
    return ret;
}


VEC_TMPL T npj::dot(const VEC_TYPE &a, const VEC_TYPE &b)
{ return a.dot(b); }

VEC_TMPL
T npj::magnitude(const VEC_TYPE &v) {return v.length();}

VEC_TMPL
T npj::length(const VEC_TYPE &v) {return v.length();}

VEC_TMPL
T npj::norm(const VEC_TYPE &v) {return v.length();}

VEC_TMPL
T npj::squaredLength(const VEC_TYPE &v) {return v.dot(v);}

VEC_TMPL
T npj::lengthSquared(const VEC_TYPE &v) {return v.dot(v);}

VEC_TMPL
T npj::sqlen(const VEC_TYPE &v) {return v.dot(v);}

VEC_TMPL
T npj::minValue(const VEC_TYPE &v) {return v.minValue();}

VEC_TMPL
T npj::maxValue(const VEC_TYPE &v) {return v.maxValue();}

VEC_TMPL
VEC_TYPE& npj::normalize(const VEC_TYPE &v, const T& len) {return v.normalize(len);}

VEC_TMPL
constexpr T VEC_TYPE::magnitude() const {return length();}

VEC_TMPL
constexpr T VEC_TYPE::norm() const {return length();}

VEC_TMPL
constexpr T VEC_TYPE::length() const { return sqrt(this->dot(*this)); }

VEC_TMPL
constexpr T VEC_TYPE::squaredLength() const {return sqlen();}

VEC_TMPL
constexpr T VEC_TYPE::lengthSquared() const {return sqlen();}

VEC_TMPL
constexpr T VEC_TYPE::sqlen() const {return this->dot(*this);}

VEC_TMPL
constexpr T VEC_TYPE::minValue() const {
    T r = (*this)[0];

    for (int i = 1; i < S; ++i)
        if (r < (*this)[i])
            r = (*this)[i];

    return r;
}

VEC_TMPL
constexpr T VEC_TYPE::maxValue() const {
    T r = (*this)[0];

    for (int i = 1; i < S; ++i)
        if (r > (*this)[i])
            r = (*this)[i];

    return r;
}


VEC_TMPL constexpr
VEC_TYPE & VEC_TYPE::normalize(const T& len) {
    if (len < 0)
        return *this /= length();
    else
        return *this = *this / length() * len;
}

VEC_TMPL constexpr
VEC_TYPE VEC_TYPE::cross(const Vector &other) const requires (S == 3)
{
    return {
        (*this)[1] * other[2] - (*this)[2] * other[1],
        (*this)[2] * other[0] - (*this)[0] * other[2],
        (*this)[0] * other[1] - (*this)[1] * other[0]
    };
}

VEC_TMPL constexpr
VEC_TYPE VEC_TYPE::lerp(const Vector &other, T ratio) const
{
    Vector ret;
    for (size_t i = 0; i < S; ++i)
        ret[i] = (*this)[i] * (1 - ratio) + other[i] * ratio;

    return ret;
}

VEC_TMPL
VEC_TYPE VEC_TYPE::zero()
{
    VEC_TYPE ret;
    
    for (int i = 0; i < S; ++i)
        ret[i] = 0;

    return ret;
}

VEC_TMPL
VEC_TYPE npj::relative(const VEC_TYPE &a, const VEC_TYPE &b) { return a - b;}
    
VEC_TMPL
VEC_TYPE npj::radial(const VEC_TYPE &vec, const VEC_TYPE &dir, const bool dirNormalized)
{
    VEC_TYPE normalized = (dirNormalized ? dir : dir.normalize());
    return vec.dot(dir) * dir;
}
    
VEC_TMPL
VEC_TYPE npj::lateral(const VEC_TYPE &vec, const VEC_TYPE &dir, const bool dirNormalized)
{
    VEC_TYPE radial = npj::radial(vec, dir, dirNormalized);
    return vec - radial;
}

VEC_TMPL
VEC_TYPE npj::cross(const VEC_TYPE &a, const VEC_TYPE &b) requires (S == 3)
{
    return a.cross(b);
}

//VEC_TMPL_OPEN>
//VEC_TYPE npj::min(std::initializer_list<VEC_TYPE> vecs)
//{
//    bool init = true;
//    T rlen;
//    VEC_TYPE ret;
//
//    for (auto& v : vecs)
//    {
//        if (init)
//        {
//            init = false;
//            ret = v;
//            rlen = v.sqlen();
//            continue;
//        }
//        T len = v.sqlen();
//        if (len < rlen)
//        {
//            rlen = len;
//            ret = v;
//        }
//    }
//
//    return ret;
//}
//
//VEC_TMPL_OPEN, typename CMP>
//VEC_TYPE npj::min(std::initializer_list<VEC_TYPE> vecs, CMP cmp)
//{
//    bool init = true;
//    VEC_TYPE ret;
//
//    for (auto& v : vecs)
//    {
//        if (init)
//        {
//            init = false;
//            ret = v;
//            continue;
//        }
//        if (cmp(v, ret))
//            ret = v;
//    }
//
//    return ret;
//}
//
//VEC_TMPL_OPEN>
//VEC_TYPE npj::min(std::initializer_list<VEC_TYPE> vecs, std::function<bool(VEC_TYPE, VEC_TYPE)> cmp)
//{
//    bool init = true;
//    VEC_TYPE ret;
//
//    for (auto& v : vecs)
//    {
//        if (init)
//        {
//            init = false;
//            ret = v;
//            continue;
//        }
//        if (cmp(v, ret))
//            ret = v;
//    }
//
//    return ret;
//}
//
//
//VEC_TMPL_OPEN>
//VEC_TYPE npj::max(std::initializer_list<VEC_TYPE> vecs)
//{
//    bool init = true;
//    T rlen;
//    VEC_TYPE ret;
//
//    for (auto& v : vecs)
//    {
//        if (init)
//        {
//            init = false;
//            ret = v;
//            rlen = v.sqlen();
//            continue;
//        }
//        T len = v.sqlen();
//        if (len > rlen)
//        {
//            rlen = len;
//            ret = v;
//        }
//    }
//
//    return ret;
//}
//
//VEC_TMPL_OPEN, typename CMP>
//VEC_TYPE npj::max(std::initializer_list<VEC_TYPE> vecs, CMP cmp)
//{
//    bool init = true;
//    VEC_TYPE ret;
//
//    for (auto& v : vecs)
//    {
//        if (init)
//        {
//            init = false;
//            ret = v;
//            continue;
//        }
//        if (cmp(v, ret))
//            ret = v;
//    }
//
//    return ret;
//}
//
//VEC_TMPL_OPEN>
//VEC_TYPE npj::max(std::initializer_list<VEC_TYPE> vecs, std::function<bool(VEC_TYPE, VEC_TYPE)> cmp)
//{
//    bool init = true;
//    VEC_TYPE ret;
//
//    for (auto& v : vecs)
//    {
//        if (init)
//        {
//            init = false;
//            ret = v;
//            continue;
//        }
//        if (cmp(v, ret))
//            ret = v;
//    }
//
//    return ret;
//}
//
//
//VEC_TMPL
//VEC_TYPE npj::lerp(const VEC_TYPE &a, const VEC_TYPE &b, T ratio)
//{
//    return a.lerp(b, ratio);
//}


#undef VEC_TEMPL
#endif
