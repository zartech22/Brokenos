
void main(void)
{
	char *msg1 = "task2: Hello world !\n";
	char *msg2 = "task2: Bye !\n";
	int i;

	asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (msg1));
	for(i=0;i<10000000;i++);
	asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (msg2));
	asm("mov $0x02, %eax; int $0x30");
}

