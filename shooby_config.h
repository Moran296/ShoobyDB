#ifndef __SHOOBY_CONFIG_H__
#define __SHOOBY_CONFIG_H__

#ifndef SHOOBY_DEBUG_PRINT
#define SHOOBY_DEBUG_PRINT(...) void(0)
#endif

#ifndef SHOOBY_ASSERT

#include <cassert>

#define SHOOBY_ASSERT(BOOL, ...) assert(BOOL)
#endif

#ifndef ON_GET_SET_TYPE_MISMATCH
#define ON_SHOOBY_TYPE_MISMATCH(...) SHOOBY_ASSERT(false, __VA_ARGS__)
#endif






#endif // __SHOOBY_CONFIG_H__
