// %%cpp div.c
// %run gcc -m32 -masm=intel -O3 div.c -S -o div.S
// %run cat div.S | grep -v "^\s*\." | grep -v "^\s*\#"

#include <stdint.h>
    
int32_t div(int32_t a) { 
    return a / 4;
}

uint32_t udiv(uint32_t a) { 
    return a / 2;
}

