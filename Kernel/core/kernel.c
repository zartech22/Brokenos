#include <utils/types.h>
#include <memory/gdt.h>
#include <video/Screen.h>
#include <core/io.h>
#include <memory/idt.h>
#include <memory/mm.h>
#include <core/process.h>
#include <utils/lib.h>
#include <memory/kmalloc.h>
#include <pci/pci.h>
#include <disk/ide.h>

void init_pic();
int main(mb_partial_info *multiboot_info);

void get_cpu_vendor(u32 str[3])
{
    asm volatile("xor %%eax, %%eax;"
				"cpuid;"
                : "=b" (*str), "=c" (*(str + 2)), "=d" (*(str + 1)) :: "%eax");
}

void get_cpu_brand(u32 str[12])
{
    asm volatile("cpuid;" : "=a" (*str), "=b" (*(str + 1)), "=c" (*(str + 2)), "=d" (*(str + 3)) : "a" (0x80000002));

    asm volatile("cpuid;" : "=a" (*(str + 4)), "=b" (*(str + 5)), "=c" (*(str + 6)), "=d" (*(str + 7)) : "a" (0x80000003));

    asm volatile("cpuid;" : "=a" (*(str + 8)), "=b" (*(str + 9)), "=c" (*(str + 10)), "=d" (*(str + 11)) : "a" (0x80000004));
}

#ifdef __cplusplus
extern "C" {
#endif
	void kmain(struct mb_partial_info*);
#ifdef __cplusplus
	}
#endif

void kmain(struct mb_partial_info *memInfo)
{
	cli;

    main(memInfo);
}

void task1()
{
	char * const msg = (char*) 0x40001000;

	msg[0] = 'T';
	msg[1] = 'a';
	msg[2] = 's';
	msg[3] = 'k';
	msg[4] = '1';
	msg[5] = '\n';
	msg[6] = 0;

    while(1)
	{
        asm("int $0x30" :: "b" (msg), "a" (0x01));
        for(int i = 0; i < 1000000; i++);
    }

	return;
}

void task2()
{
	char * const msg = (char*) 0x40001000;

	msg[0] = 'T';
	msg[1] = 'a';
	msg[2] = 's';
	msg[3] = 'k';
	msg[4] = '2';
	msg[5] = '\n';
	msg[6] = 0;

	while(1)
	{
		asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (msg));
		for(int i = 0; i < 1000000; i++);
	}

	return;
}

void task3()
{
	char * const msg = (char*) 0x40001000;

	msg[0] = 'J';
	msg[1] = 'e';
	msg[2] = ' ';
	msg[3] = 's';
	msg[4] = 'u';
	msg[5] = 'i';
	msg[6] = 's';
	msg[7] = ' ';
	msg[8] = 'T';
	msg[9] = 'a';
	msg[10] = 's';
	msg[11] = 'k';
	msg[12] = '3';
	msg[13] = '\n';
	msg[14] = 0;

	while(1)
	{
		asm("mov %0, %%ebx; mov $0x01, %%eax; int $0x30" :: "m" (msg));
		for(int i = 0; i < 1000000; i++);
	}
}

int main(struct mb_partial_info *mbinfo)
{
    init_gdt();
    // Update SS and ESP
    asm("movw $0x18, %%ax	\n \
         movw %%ax, %%ss		\n \
         movl %0, %%esp" :: "i"(KERN_STACK));

    init_idt();
    init_mm(mbinfo->high_mem);

    Screen::initScreen((struct VbeModeInfo*)mbinfo->vbe_mode_info);

    Screen &s = Screen::getScreen();

    s.printInfo("Kernel charge en memoire !");
    //s.okMsg();
    s.printInfo("kernel : chargement idt...");
    //s.okMsg();
    s.printInfo("kernel : initialisation de la memoire...");
    s.println("kernel : paging actif !");

    s.printDebug("Info memoire : %uk (lower) %uk (upper)", mbinfo->low_mem, mbinfo->high_mem);

    s.printInfo("kernel : chargement nouvelle gdt...");
//    s.okMsg();
    s.printInfo("\t->Reaffectation du registre SS et ESP...");

    struct VbeModeInfo *info = (struct VbeModeInfo*)mbinfo->vbe_mode_info;

    s.printDebug("VBE BitsPerPixel %u, %u, %u,  %p", info->XResolution, info->YResolution, info->BitsPerPixel, info->PhysBasePtr);


    if(mbinfo->flags & 0x800)
        s.printDebug("C'est good ! %s", mbinfo->boot_loader_name);
    else
        s.printDebug("La merde...");

    //displayTest(info, 'a');


    s.printInfo("kernel : configuration du PIC...");
    init_pic();
//    s.okMsg();

    s.printInfo("kernel : init tss");
    asm("	movw $0x38, %ax \n \
            ltr %ax");
//    s.okMsg();

    s.printInfo("kernel : reactivation des interruptions...");
//    s.okMsg();

    s.printInfo("Initialisation de l'horloge...");
	outb(0x43, 0x34);
	outb(0x40, (0x1234DE / 50) & 0x00FF);
    outb(0x40, (0x1234DE / 50) >> 8);
//    s.okMsg();


    s.printInfo("Kernel pret a l'action !");

    s.hide_cursor();

	//Init kernel thread
	current = &p_list[0];
	current->pid = 0;
	current->state = 1;
	current->regs.cr3 = (u32) pd0;

    //load_task((char*) &task1, 0x2000);
    /*load_task((char*) &task2, 0x2000);*/

	/*** TEST DISK ***/

	/*char *msg = "Hello world\n";
	char *buf;

	buf = (char*) kmalloc(512);

	memset(buf, 0, 512);

	memcpy((char*) buf, (char*) msg, strlen(msg) + 1);
	printInfo("Test ecriture sur disque");
	bl_write(1, 2, 1, buf);

	char *buf = (char*)kmalloc(512);
	memset(buf, 0, 512);

	bl_read(1, 2, 1, buf);

	printInfo("Test lecture sur disque");
	printk("buf : %s\n", buf);//*/

	char *vendor = (char*) kmalloc(13);
	memset(vendor, 0, 13);

	char *brand = (char*) kmalloc(49);
	memset(brand, 0, 49);


	get_cpu_vendor((u32*) vendor);
	get_cpu_brand((u32*) brand);

    s.println("Vendor : %s", vendor);
	kfree(vendor);

    s.println("Brand : %s", brand);
	kfree(brand);

    s.printDebug("PCI list :");
	pciGetVendors();

    IdeDrive &d = ctrl.getDrive(PrimaryBus, Master);

	/*char *buffer = (char*)kmalloc(512);
	memset(buffer, 0, 512);
	strcpy(buffer, "Salut, je suis un petit test !");

	d.write(2, 1, buffer);*/

    d.displayPartitions();

    s.printInfo("kernel : tasks created");

    s.printInfo("kernel : scheduler enabled");

    s.printError("Address : %p", info->PhysBasePtr);

    sti;

	for(;;)
		asm("hlt");
}
