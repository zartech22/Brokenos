#include "pic.h"

#include "io.h"

namespace kernel::core::pic {
    void init() {
        //init ICW1
        outb(0x20, 0x11)
        outb(0xA0, 0x11)

        //init ICW2
        outb(0x21, 0x20)    //vec de depart : 32
        outb(0xA1, 0x70)    //vec de depart : 96

        //init ICW3
        outb(0x21, 0x04)
        outb(0xA1, 0x02)

        //init ICW4
        outb(0x21, 0x01)
        outb(0xA1, 0x01)

        //masquage des interrupt
        outb(0x21, 0x0)
        outb(0xA1, 0x0)
    }
}