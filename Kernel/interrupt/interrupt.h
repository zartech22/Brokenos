#ifdef __cplusplus
	extern "C" {
#endif
	void isr_default_int();
	void isr_default_exc();
    void isr_GP_exc(u32 error);
    void isr_PF_exc(u32 error);
	void isr_clock_int();
	void isr_kbd_int();
	void do_syscall(int);
#ifdef __cplusplus
	}
#endif
