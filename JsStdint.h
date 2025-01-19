#ifndef __JSSTDINT_H__
#define __JSSTDINT_H__

#ifdef _MSC_VER
#if _MSC_VER < 1600 // MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)

#ifndef __JSSTDINT_TYPES__
#define __JSSTDINT_TYPES__
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif /* __JSSTDINT_TYPES__ */

#else
#include <stdint.h>
#endif /* _MSC_VER < 1600 */
#else
#include <stdint.h>
#endif /* _MSC_VER */

#endif /* __JSSTDINT_H__ */