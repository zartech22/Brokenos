#pragma once

#include <utils/String.h>

namespace kernel::core::cpu {
    inline String getVendor()
    {
        auto vendor = new uint32_t[13];

        asm ("xor %%eax, %%eax;"
             "cpuid;"
             : "=b" (*vendor), "=c" (*(vendor + 2)), "=d" (*(vendor + 1)) :: "%eax");

        vendor[12] = '\0';

        String res(reinterpret_cast<char *>(vendor));

        delete vendor;

        return res;
    }

    inline String getBrand()
    {
        auto brand = new uint32_t[49];

        asm ("cpuid;" : "=a" (*brand), "=b" (*(brand + 1)), "=c" (*(brand + 2)), "=d" (*(brand + 3)) : "a" (0x80000002));
        asm ("cpuid;" : "=a" (*(brand + 4)), "=b" (*(brand + 5)), "=c" (*(brand + 6)), "=d" (*(brand + 7)) : "a" (0x80000003));
        asm ("cpuid;" : "=a" (*(brand + 8)), "=b" (*(brand + 9)), "=c" (*(brand + 10)), "=d" (*(brand + 11)) : "a" (0x80000004));

        brand[48] = '\0';

        String res(reinterpret_cast<char *>(brand));

        delete brand;

        return res;
    }
}
