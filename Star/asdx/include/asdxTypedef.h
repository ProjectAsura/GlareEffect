﻿//-------------------------------------------------------------------------------------------
// File : asdxTypedef.h
// Desc : Type definitions.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------

#ifndef __ASDX_TYPEDEF_H__
#define __ASDX_TYPEDEF_H__

#if defined(WIN32) || defined(_WIN32)
#ifndef ASDX_IS_WIN
#define ASDX_IS_WIN     (1)
#endif//ASDX_IS_WIN
#endif

#ifndef ASDX_INLINE
    #ifdef  _MSC_VER
        #if (_MSC_VER >= 1200)
            #define ASDX_INLINE __forceinline
        #else
            #define ASDX_INLINE __inline
        #endif//(_MSC_VER >= 1200)
    #else
        #ifdef __cplusplus
            #define ASDX_INLINE inline
        #else
            #define ASDX_INLINE 
        #endif//__cpulusplus
    #endif//_MSC_VER
#endif//ASDX_INLINE


#ifndef ASDX_TEMPLATE
#define ASDX_TEMPLATE(T)               template< typename T >
#endif//ASDX_TEMPLATE


#ifndef ASDX_TEMPLATE_INLINE
#define ASDX_TEMPLATE_INLINE(T)            ASDX_TEMPLATE(T) ASDX_INLINE
#endif//ASDX_TEMPLATE_INLINE


#ifndef ASDX_UNUSED_VAR
#define ASDX_UNUSED_VAR(x)             ((void*)&x)
#endif//ASDX_UNUSED_VAR


#ifndef ASDX_API
    #if ASDX_IS_WIN
        #ifndef ASDX_EXPORTS
            #define ASDX_API       __declspec( dllexport )
        #else
            #define ASDX_API       __declspec( dllimport )
        #endif//ASDX_EXPORTS
    #endif//ASDX_IS_WIN
#endif//ASDX_API


#ifndef ASDX_APIENTRY
    #if defined(WIN32) || defined(_WIN32)
        #define ASDX_APIENTRY      __stdcall
    #else
        #define ASDX_APIENTRY
    #endif// defined(WIN32) || defined(_WIN32)
#endif//ASDX_APIENTRY


#ifndef ASDX_ALIGN
    #if _MSC_VER
        #define ASDX_ALIGN( alignment )    __declspec( align(alignment) )
    #else
        #define ASDX_ALIGN( alignment )    __attribute__( aligned(alignment) )
    #endif
#endif//ASDX_ALIGN


#ifndef ASDX_DELETE
#define ASDX_DELETE(p)         { if (p) { delete (p); (p) = nullptr; } }
#endif//ASDX_DELETE

#ifndef ASDX_DELETE_ARRAY
#define ASDX_DELETE_ARRAY(p)   { if (p) { delete [] (p); (p) = nullptr; } }
#endif//ASDX_DELETE_ARRAY

#ifndef ASDX_RELEASE
#define ASDX_RELEASE(p)        { if (p) { (p)->Release(); (p) = nullptr; } }
#endif//ASDX_RELEASE


//-------------------------------------------------------------------------
// Type Defenition
//-------------------------------------------------------------------------

//=========================================================================
//! @ingroup
//! @defgroup DataTypes     Data Definition
//! @{

//-------------------------------------------------------------------------
//! @typedef    u8
//! @brief      符号無し8bit整数です.
//! @note       書式指定は "%hhu" です.
//-------------------------------------------------------------------------
typedef unsigned char       u8;

//-------------------------------------------------------------------------
//! @typedef    u16
//! @brief      符号無し16bit整数です.
//! @note       書式指定は "%hu" です.
//-------------------------------------------------------------------------
typedef unsigned short      u16;

//-------------------------------------------------------------------------
//! @typedef    u32
//! @brief      符号無し32bit整数です.
//! @note       書式指定は "%u" です.
//-------------------------------------------------------------------------
typedef unsigned int        u32;

//-------------------------------------------------------------------------
//! @typedef    u64
//! @brief      符号無し64bit整数です.
//! @note       書式指定は "%llu" です.
//-------------------------------------------------------------------------
#if ASDX_IS_WIN
typedef unsigned __int64    u64;
#else
typedef unsigned long long  u64;
#endif

//-------------------------------------------------------------------------
//! @typedef    s8
//! @brief      符号付き8bit整数です.
//! @note       書式指定は "%hhd" です.
//-------------------------------------------------------------------------
typedef signed char         s8;

//-------------------------------------------------------------------------
//! @typedef    s16
//! @brief      符号付き16bit整数です.
//! @note       書式指定は "%hd" です.
//-------------------------------------------------------------------------
typedef signed short        s16;

//-------------------------------------------------------------------------
//! @typedef    s32
//! @brief      符号付き32bit整数です.
//! @note       書式指定は "%d" です.
//-------------------------------------------------------------------------
typedef signed int          s32;

//-------------------------------------------------------------------------
//! @typedef    s64
//! @brief      符号付き64bit整数です.
//! @note       書式指定は "%lld" です.
//-------------------------------------------------------------------------
#if ASDX_IS_WIN
typedef signed __int64      s64;
#else
typedef signed long long    s64;
#endif

//-------------------------------------------------------------------------
//! @typedef    f16
//! @brief      半精度(16bit)浮動小数です.
//-------------------------------------------------------------------------
typedef unsigned short      f16;

//-------------------------------------------------------------------------
//! @typedef    f32
//! @brief      単精度(32bit)浮動小数です.
//! @note       書式指定は "%f" です.
//-------------------------------------------------------------------------
typedef float               f32;

//-------------------------------------------------------------------------
//! @typedef    f64
//! @brief      倍精度(64bit)浮動小数です.
//! @note       書式指定は "%lf" です.
//-------------------------------------------------------------------------
typedef double              f64;

//-------------------------------------------------------------------------
//! @typedef    char8
//! @brief      8bit文字です.
//-------------------------------------------------------------------------
typedef char                char8;

//-------------------------------------------------------------------------
//! @typedef    char16
//! @brief      16bit文字です.
//-------------------------------------------------------------------------
typedef wchar_t             char16;


//-------------------------------------------------------------------------
//! @def        S8_MIN
//! @brief      符号付き8bit整数型の最小値です.
//-------------------------------------------------------------------------
#ifndef S8_MIN
#define S8_MIN          (-127i8 - 1)
#endif//S8_MIN

//-------------------------------------------------------------------------
//! @def        S16_MIN
//! @brief      符号付き16bit整数型の最小値です.
//-------------------------------------------------------------------------
#ifndef S16_MIN
#define S16_MIN         (-32767i16 - 1)
#endif//S16_MIN

//-------------------------------------------------------------------------
//! @def        S32_MIN
//! @brief      符号付き32bit整数型の最小値です.
//-------------------------------------------------------------------------
#ifndef S32_MIN
#define S32_MIN         (-2147483647i32 - 1)
#endif//S32_MIN

//-------------------------------------------------------------------------
//! @def        S64_MIN
//! @brief      符号付き64bit整数型の最小値です.
//-------------------------------------------------------------------------
#ifndef S64_MIN
#define S64_MIN         (-9223372036854775807i64 - 1)
#endif//S64_MIN

//-------------------------------------------------------------------------
//! @def        S8_MAX
//! @brief      符号付8bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef S8_MAX
#define S8_MAX          127i8
#endif//S8_MAX

//-------------------------------------------------------------------------
//! @def        S16_MAX
//! @brief      符号付き16bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef S16_MAX
#define S16_MAX         32767i16
#endif//S16_MAX

//-------------------------------------------------------------------------
//! @def        S32_MAX
//! @brief      符号付き32bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef S32_MAX
#define S32_MAX         2147483647i32
#endif//S32_MAX

//-------------------------------------------------------------------------
//! @def        S64_MAX
//! @brief      符号付き64bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef S64_MAX
#define S64_MAX         9223372036854775807i64
#endif//S64_MAX

//-------------------------------------------------------------------------
//! @def        U8_MAX
//! @brief      符号無し8bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef U8_MAX
#define U8_MAX          0xffui8
#endif//U8_MAX

//-------------------------------------------------------------------------
//! @def        U16_MAX
//! @brief      符号無し16bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef U16_MAX
#define U16_MAX         0xffffui16
#endif//U16_MAX

//-------------------------------------------------------------------------
//! @def        U32_MAX
//! @brief      符号無し32bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef U32_MAX
#define U32_MAX         0xffffffffui32
#endif//U32_MAX

//-------------------------------------------------------------------------
//! @def        U64_MAX
//! @brief      符号無し64bit整数型の最大値です.
//-------------------------------------------------------------------------
#ifndef U64_MAX
#define U64_MAX         0xffffffffffffffffui64
#endif//U64_MAX

//-------------------------------------------------------------------------
//! @def        S64_C
//! @brief      値xをs64型の符号付き整数定数式へ展開します.
//-------------------------------------------------------------------------
#ifndef S64_C
#define S64_C(x)        ( (x) + ( S64_MAX - S64_MAX ) )
#endif//S64_C

//-------------------------------------------------------------------------
//! @def        U64_C
//! @brief      値xをu64型の符号無し整数定数式へ展開します.
//-------------------------------------------------------------------------
#ifndef U64_C
#define U64_C(x)        ( (x) + ( U64_MAX - U64_MAX ) )
#endif//U64_C

//-------------------------------------------------------------------------
//! @def        F16_MIN
//! @brief      半精度(16bit)浮動小数型の最小値です.
//-------------------------------------------------------------------------
#ifndef F16_MIN
#define F16_MIN             5.96046448e-08F
#endif//F16_MIN

//-------------------------------------------------------------------------
//! @def        F32_MIN
//! @brief      単精度(32bit)浮動小数型の最小値です.
//-------------------------------------------------------------------------
#ifndef F32_MIN
#define F32_MIN             1.175494351e-38F
#endif//F32_MIN

//-------------------------------------------------------------------------
//! @def        F64_MIN
//! @brief      倍精度(64bit)浮動小数型の最小値です.
//-------------------------------------------------------------------------
#ifndef F64_MIN
#define F64_MIN             2.2250738585072014e-308
#endif//F64_MIN

//-------------------------------------------------------------------------
//! @def        F16_MAX
//! @brief      半精度(16bit)浮動小数型の最大値です.
//-------------------------------------------------------------------------
#ifndef F16_MAX
#define F16_MAX             65504.0f
#endif//F16_MAX

//-------------------------------------------------------------------------
//! @def        F32_MAX
//! @brief      単精度(32bit)浮動小数型の最大値です.
//-------------------------------------------------------------------------
#ifndef F32_MAX
#define F32_MAX             3.402823466e+38F
#endif//F32_MAX

//-------------------------------------------------------------------------
//! @def        F64_MAX
//! @brief      倍精度(64bit)浮動小数型の最大値です.
//-------------------------------------------------------------------------
#ifndef F64_MAX
#define F64_MAX             1.7976931348623158e+308
#endif//F64_MAX

//-------------------------------------------------------------------------
//! @def        F16_EPSILON
//! @brief      半精度(16bit)浮動小数型で表現可能な1より大きい最小値と1との差.
//-------------------------------------------------------------------------
#ifndef F16_EPSILON
#define F16_EPSILON         0.00097656f
#endif//F16_EPSILON

//-------------------------------------------------------------------------
//! @def        F32_EPSILON
//! @brief      単精度(32bit)浮動小数型で表現可能な1より大きい最小値と1との差.
//-------------------------------------------------------------------------
#ifndef F32_EPSILON
#define F32_EPSILON         1.192092896e-07F
#endif//F32_EPSILON

//-------------------------------------------------------------------------
//! @def        F64_EPSILON
//! @brief      倍精度(64bit)浮動小数型で表現可能な1より大きい最小値と1との差.
//-------------------------------------------------------------------------
#ifndef F64_EPSILON
#define F64_EPSILON         2.2204460492503131e-016
#endif//F64_EPSILON

//! @}
//=========================================================================


#endif//__ASDX_TYPEDEF_H__
