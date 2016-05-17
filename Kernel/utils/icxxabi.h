#ifndef _ICXXABI_H
#define _ICXXABI_H

#include <stdint.h>

#define ATEXIT_MAX_FUNCS	128

#if UINT32_MAX == UINTPTR_MAX
    #define STACK_CHK_GUARD 0xE2DEE396
#else
    #define STACK_CHK_GUARD 0x595E9FBD94FDA766
#endif

#ifdef __cplusplus
extern "C" {
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;
 
typedef unsigned uarch_t;
 
struct atexit_func_entry_t
{
	/*
	* Each member is at least 4 bytes large. Such that each entry is 12bytes.
	* 128 * 12 = 1.5KB exact.
	**/
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
};
 
int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);
void __cxa_pure_virtual();
void __stack_chk_fail();
 
#ifdef __cplusplus
};
#endif
 
#endif
