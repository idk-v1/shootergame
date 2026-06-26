// File: BaseInjection.h
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

#ifndef NPJ_BASE_INJECTION_H
#define NPJ_BASE_INJECTION_H


struct NULL_BASE{};
#define NPJ_WRAP(...) __VA_ARGS__
#define NPJ_CONCAT(A,B) A##B
#define COMMA ,

#define NPJ_BINJ_DEF_BASE(BASE, ...) : BASE __VA_ARGS__
#define NPJ_MAKE_TEMPLATE(...) template<__VA_ARGS__>
#define NPJ_BINJ_STRUCT_OPEN(NAME, BASE, ...) NPJ_MAKE_TEMPLATE(__VA_ARGS__) struct NAME BASE {
#define NPJ_BINJ_STRUCT_CLOSE };

#define NPJ_BINJ_STRUCT_COMP(NAME, BASE, BODY, ...)         \
    NPJ_BINJ_STRUCT_OPEN(NAME, BASE, __VA_ARGS__)           \
    BODY \
    NPJ_BINJ_STRUCT_CLOSE

#define NPJ_MAKE_BINJ_NAME_CHOICE(KEY, INDEX) NPJ_BINJ_CHOICE_##KEY##_##INDEX
#define NPJ_MAKE_BINJ_NAME_CHOICE_END(KEY) NPJ_BINJ_CHOICE_##KEY##_END

#define NPJ_MAKE_BINJ_NAME(KEY, INDEX) NPJ_BINJ_NAME_##KEY##_##INDEX
#define NPJ_MAKE_BINJ_NAME_COND(KEY, INDEX) NPJ_BINJ_NAME_##KEY##_##INDEX##_COND
#define NPJ_MAKE_BINJ_NAME_END_COND(KEY) NPJ_BINJ_NAME_##KEY##_END_COND


#endif
