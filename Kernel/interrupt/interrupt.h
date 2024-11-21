#pragma once

#include <utils/types.h>

#ifdef __cplusplus
	extern "C" {
#endif
	void isr_default_int();
	void isr_default_exc();
    void isr_GP_exc();
    void isr_PF_exc();
	void isr_clock_int();
	void isr_kbd_int();
	void do_syscall(int);
#ifdef __cplusplus
	}
#endif

#ifdef _TIME_
inline uint64_t msTime = 0;
#else
    extern uint64_t msTime;
#endif
