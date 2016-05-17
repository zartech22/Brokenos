all:
	$(MAKE) -C Kernel
	#$(MAKE) -C user

clean:
	$(MAKE) -C Kernel clean
	#$(MAKE) -C user clean
