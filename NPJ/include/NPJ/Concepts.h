// File: Concepts.h
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

#ifndef INTREPID_PROJECTS_COMMON_CONCEPTS_H
#define INTREPID_PROJECTS_COMMON_CONCEPTS_H

namespace npj
{
    template <typename T = bool> inline constexpr bool always_false_v = false;
    template <typename T = bool> inline constexpr bool always_true_v  = true;
    template <typename T> concept       Addable = requires(T a, T b) {{ a  + b } -> std::same_as<T>;};
    template <typename T> concept  Subtractable = requires(T a, T b) {{ a  - b } -> std::same_as<T>;};
    template <typename T> concept  Multipliable = requires(T a, T b) {{ a  * b } -> std::same_as<T>;};
    template <typename T> concept     Dividable = requires(T a, T b) {{ a  / b } -> std::same_as<T>;};
    template <typename T> concept       Modable = requires(T a, T b) {{ a  % b } -> std::same_as<T>;};
    template <typename T> concept AddAssignable = requires(T a, T b) {{ a += b } -> std::same_as<T&>;};
    template <typename T> concept SubAssignable = requires(T a, T b) {{ a -= b } -> std::same_as<T&>;};
    template <typename T> concept MulAssignable = requires(T a, T b) {{ a *= b } -> std::same_as<T&>;};
    template <typename T> concept DivAssignable = requires(T a, T b) {{ a /= b } -> std::same_as<T&>;};
    template <typename T> concept ModAssignable = requires(T a, T b) {{ a %= b } -> std::same_as<T&>;};
    template <typename T> concept    Assignable = requires(T a, T b) {{ a  = b } -> std::same_as<T&>;};
    template <size_t M, size_t S> concept MinSize = M <= S;
    template <size_t M, size_t S> concept MaxSize = M >= S;
    template <size_t M, size_t S> concept  IsSize = M == S;


    // @todo add error message if not derived from
    template <typename Base, typename T> concept DerivedFrom = std::derived_from<Base, T>;
}

#endif
