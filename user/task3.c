
void main(void)
{
	char *msg1 = "task3: Yeah !\n";
	char *msg2 = "task3: Bye !\n";
	int i, j;

	for(i=0;i<5;i++) {
		asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (msg1));
		for (j=0;j<5000000;j++) ;
	}
	asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (msg2));
	asm("mov $0x02, %eax; int $0x30");
}

