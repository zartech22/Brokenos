all:
	$(MAKE) -C Kernel
	$(MAKE) -C user

clean:
	$(MAKE) -C Kernel clean
	#$(MAKE) -C user clean

deploy: all
	SUDO_ASKPASS=/usr/bin/ssh-askpass sudo -A cp Kernel/kernel /media/kevin/3842f66d-4707-4554-a3e4-e6fd7c78dcbd/boot/kernel
	sync
	SUDO_ASKPASS=/usr/bin/ssh-askpass sudo -A VBoxManage startvm BrokenOS