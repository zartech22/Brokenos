#include <utils/types.h>
#include <video/Screen.h>
#include <interrupt/kbd.h>
#include <core/io.h>
#include <interrupt/scheduler.h>
#define _TIME_
#include <interrupt/interrupt.h>
#include <core/process.h>
#include <memory/kmalloc.h>
#include <memory/mm.h>


void isr_default_int() {
    //sScreen.print("An interrupt has been catch !\n");
}

void isr_default_exc() {
    sScreen.printError("Division by 0 !");
}

void isr_clock_int() {
    static int tic = 0;
    static int sec = 0;

    msTime += 20;

    tic++;

    if (tic % 50 == 0) {
        sec++;
        tic = 0;

        if (sScreen.isLoading())
            sScreen.showTic();
        else
            sScreen.putcar('.');
    }
    schedule();
}

void isr_GP_exc() {
    uint32_t fault_addr;
    uint32_t error;
    auto opcode = (char *) fault_addr;

    asm("movl 44(%%esp), %%eax;"
        "movl %%eax, %0" : "=m"(error) :);

    /*asm("movl 60(%%ebp), %%eax;"
        "mov %%eax, %0;" : "=m"(fault_addr):);*/

    //sScreen.setPos(0, 0);
    //sScreen.clean();
    sScreen.printError("#GP");
    sScreen.printError("Faulting address : %p", fault_addr);

    if (error != 0) {
        sScreen.printError("Error's origin : %s", (error & 0x1) ? "External" : "Internal");

        switch ((error >> 1) & 0x3) {
            case 0x0:
                sScreen.printError("GDT Selector");
                break;
            case 0x1:
                sScreen.printError("IDT Selector");
                break;
            case 0x2:
                sScreen.printError("LDT Selector");
                break;
            case 0x3:
                sScreen.printError("IDT Selector");
                break;
            default:
                break;
        }

        sScreen.printError("Selector : %x", (error >> 3) & 0x1FFF);
        sScreen.printError("Error : %b", error);
    } else
        sScreen.printError("Unknown error");

    asm("hlt");
}

void isr_PF_exc() {
    uint32_t faulting_addr;
    uint32_t eip;
    uint32_t error;

    asm("	movl 60(%%ebp), %%eax;	\
			mov %%eax, %0;			\
			mov %%cr2, %%eax;		\
            mov %%eax, %1;          \
            movl 44(%%esp), %%eax;  \
            movl %%eax, %2;": "=m"(eip), "=m"(faulting_addr), "=m"(error): );

    //sScreen.setPos(0, 0);
    //sScreen.clean();
    sScreen.printError("#PF");
    sScreen.printError("Faulting addr %p, EIP %p", faulting_addr, eip);

    sScreen.printError("Error desc :");

    sScreen.printError("%s", (error & 0x1) ? "Protection violation" : "Non-present Page");
    sScreen.printError("Caused by %s access", (error & 0x2) ? "write" : "read");
    sScreen.printError("In %s mode", (error & 0x4) ? "user" : "root");

    if (error & 0x8)
        sScreen.printError("Page entry with reserved bit(s) set");
    if (error & 0x10)
        sScreen.printError("Fault caused by instruction fetch");


    asm("hlt");
}

void isr_kbd_int() {
    uint8_t i;
    static int lshift_enable;
    static int rshift_enable;
    static int alt_enable;
    static int ctrl_enable;

    do {
        i = inb(0x64);
    } while ((i & 0x01) == 0);

    i = inb(0x60);
    i--;

    sScreen.printError("%x", i);

    if (i < 0x80) {
        switch (i) {
            case 0x29:
                lshift_enable = 1;
                break;
            case 0x35:
                rshift_enable = 1;
                break;
            case 0x1C:
                ctrl_enable = 1;
                break;
            case 0x37:
                alt_enable = 1;
                break;
            default:
                sScreen.printInfo("%x", i * 4 + (lshift_enable || rshift_enable));
                sScreen.printInfo("%x", kbdmap[i * 4 + (lshift_enable || rshift_enable)]);
                sScreen.putcar(kbdmap[i * 4 + (lshift_enable || rshift_enable)]);
        }
    } else {
        i -= 0x80;

        switch (i) {
            case 0x29:
                lshift_enable = 0;
                break;
            case 0x35:
                rshift_enable = 0;
                break;
            case 0x1C:
                ctrl_enable = 0;
                break;
            case 0x37:
                alt_enable = 0;
                break;
        }
    }

    /*if(kbdmap[i * 4] == 'l')
        sScreen.showLoadScreen();*/
}

void do_syscall(int sys_num) {
    if (sys_num == 1) {
        char *u_str;

        asm("" : "=b"(u_str) :);
        for (int i = 0; i < 10000; i++); //temporisation
        cli;
        sScreen.print(u_str);
        sti;
    } else if (sys_num == 2) {
        uint16_t kss;
        uint32_t kesp;

        page_list *pl, *oldpl;

        cli;

        n_proc--;
        current->state = 0;

        pl = current->pglist;

        while (pl) {
            release_page_frame(pl->page->p_addr);
            kfree(pl->page);
            oldpl = pl;
            pl = pl->next;
            kfree(oldpl);
        }

        release_page_frame((uint32_t) get_p_addr((char*) USER_STACK));

        kss = p_list[0].regs.ss;
        kesp = p_list[0].regs.esp;

        asm("mov %0, %%ss;"
            "mov %1, %%esp;" :: "m"(kss), "m"(kesp));

        release_page_from_heap(reinterpret_cast<char *>(current->kstack.esp0 & 0xFFFFF000));

        asm("mov %0, %%eax;"
            "mov %%eax, %%cr3" :: "m"(pd0));

        switch_to_task(0, ExecutionMode::KERNEL_MODE);
    } else
        sScreen.printError("Unknown syscall !");
}
