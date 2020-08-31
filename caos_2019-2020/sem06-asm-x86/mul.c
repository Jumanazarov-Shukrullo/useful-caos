// %%cpp mul.c
// %run gcc -m64 -masm=intel -O3 mul.c -S -o mul.S
// %run cat mul.S | grep -v "^\s*\."

#include <stdint.h>
    
int32_t mul(int32_t a) { 
    return a * 14;
}

