#ifndef __SHOOBY_CONFIG_H__
#define __SHOOBY_CONFIG_H__

#ifndef SHOOBY_DEBUG_PRINT
#define SHOOBY_DEBUG_PRINT(...) void(0)
#endif

// ASSERT IMPLEMENTATION
#ifndef SHOOBY_ASSERT
#include <cassert>
#define SHOOBY_ASSERT(BOOL, ...) assert(BOOL)
#endif

// BEHAVIOR ON GET/SET TYPE MISMATCH
#ifndef ON_SHOOBY_TYPE_MISMATCH
#define ON_SHOOBY_TYPE_MISMATCH(...) SHOOBY_ASSERT(false, __VA_ARGS__)
#endif

// MUTEX RELATED IMPLEMENTATION
#ifndef SHOOBY_MUTEX_TYPE
#include <mutex>
#define SHOOBY_MUTEX_TYPE std::mutex
#define SHOOBY_MUTEX_INIT(m)
#define SHOOBY_LOCK(m) m.lock()
#define SHOOBY_UNLOCK(m) m.unlock()
#else
#if !defined(SHOOBY_MUTEX_INIT) || !defined(SHOOBY_LOCK) || !defined(SHOOBY_UNLOCK)
#error "SHOOBY_MUTEX_TYPE is defined but SHOOBY_MUTEX_INIT or SHOOBY_LOCK/UNLOCK is not defined"
#endif
#endif

#endif // __SHOOBY_CONFIG_H__
