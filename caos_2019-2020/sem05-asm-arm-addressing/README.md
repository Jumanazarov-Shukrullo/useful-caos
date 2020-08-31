


# Адресация памяти в ассемблере

[Ридинг от Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/arm_load_store)

Добавление от меня:

1. `str r0, [r1, #4]! (C-style: *(r1 += 4) = r0)` - то же самое, что и `str r0, [r1, #4] (C-style: *(r1 + 4) = r0)`, но в `r1`, будет сохранено `r1 + #4` после выполнения команды. Другими словами префиксный инкремент на 4.
1. `ldr r0, [r1], #4` - то же самое, что и `ldr r0, [r1] (C-style: r0 = *r1)` с последующим `add r1, r1, #4 (C-style: r1 += 4)`. Другими словами постфиксный инкремент.


```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/Downloads/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/"

```

# Пример работы с массивом из ассемблера


```cpp
%%cpp is_sorted.c
%run arm-linux-gnueabi-gcc -marm is_sorted.c -o is_sorted.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./is_sorted.exe

#include <stdio.h>
#include <assert.h>

int is_sorted(int n, unsigned int* x);
__asm__ (R"(
.global is_sorted
is_sorted:
    // r0 - n, r1 - x
    cmp r0, #1
    bls is_sorted_true
    sub r0, r0, #1
    add r1, r1, #4
    ldr r2, [r1, #-4]
    ldr r3, [r1]
    cmp r2, r3
    bhi is_sorted_false
    b is_sorted
is_sorted_false:
    mov r0, #0
    bx lr
is_sorted_true:
    mov r0, #1
    bx  lr
)");

#define check(result, ...) {\
    unsigned int a[] = {__VA_ARGS__}; \
    int r = is_sorted(sizeof(a) / sizeof(int), a); \
    printf("is_sorted({" #__VA_ARGS__ "}) = %d ?= %d\n", r, result);\
    assert(r == result); \
}

#define check_sorted(...) check(1, __VA_ARGS__)
#define check_not_sorted(...) check(0, __VA_ARGS__)


int test() {
    check_sorted();
    check_sorted(10);
    check_sorted(10, 20);
    check_sorted(10, 20, 30);
    check_sorted(30, 30, 30);
    check_not_sorted(20, 10);
    check_not_sorted(10, 30, 20);
}

int main() {
    test();
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm is_sorted.c -o is_sorted.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./is_sorted.exe`


    is_sorted({}) = 1 ?= 1
    is_sorted({10}) = 1 ?= 1
    is_sorted({10, 20}) = 1 ?= 1
    is_sorted({10, 20, 30}) = 1 ?= 1
    is_sorted({30, 30, 30}) = 1 ?= 1
    is_sorted({20, 10}) = 0 ?= 0
    is_sorted({10, 30, 20}) = 0 ?= 0


# Пример приема более, чем 4 аргументов в функции

ip = r12. Почему он тут портится? У меня нет ответа `¯\_(ツ)_/¯ `


```cpp
%%cpp more_than_4.c
%run arm-linux-gnueabi-gcc -marm more_than_4.c -O2 -S -o more_than_4.s
%run cat more_than_4.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

int mega_sum(int a1, int a2, int a3, int a4, int a5, int a6) {
    return a1 + a2 + a3 + a4 + a5 + a6;
}
```


Run: `arm-linux-gnueabi-gcc -marm more_than_4.c -O2 -S -o more_than_4.s`



Run: `cat more_than_4.s | grep -v "^\\s*\\." | grep -v "^\\s*@"`


    mega_sum:
    	add	r1, r0, r1
    	ldr	ip, [sp]
    	add	r1, r1, r2
    	ldr	r0, [sp, #4]
    	add	r1, r1, r3
    	add	r1, r1, ip
    	add	r0, r1, r0
    	bx	lr


# Пример чтения структуры из ассемблера


```cpp
%%cpp cut_struct.c
%run arm-linux-gnueabi-gcc -marm cut_struct.c -o cut_struct.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./cut_struct.exe

#include <stdio.h>
#include <assert.h>

struct Obj {
    char c;
    int i;
    short s;
    char c2;
} __attribute__((packed));

int cut_struct(struct Obj* obj, char* c, int* i, short* s, char* c2);
__asm__ (R"(
.global cut_struct
cut_struct:
    push {r4, r5} // notice that we decrease sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - obj, r1 - c, r2 - i, r3 - s, r4 - c2
    ldrb r5, [r0, #0]
    strb r5, [r1]
    ldr r5, [r0, #1]
    str r5, [r2]
    ldrh r5, [r0, #5]
    strh r5, [r3]
    ldrb r5, [r0, #7]
    strb r5, [r4]
    pop {r4, r5}
    bx  lr
)");

int test() {
    // designated initializers: https://en.cppreference.com/w/c/language/struct_initialization
    struct Obj obj = {.c = 123, .i = 100500, .s = 15000, .c2 = 67};
    char c = 0; int i = 0; short s = 0; char c2 = 0; // bad codestyle
    cut_struct(&obj, &c, &i, &s, &c2);
    fprintf(stderr, "Got c=%d, i=%d, s=%d, c2=%d", (int)c, (int)i, (int)s, (int)c2);
    assert(c == obj.c && i == obj.i && s == obj.s && c2 == obj.c2);
}

int main() {
    test();
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm cut_struct.c -o cut_struct.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./cut_struct.exe`


    Got c=123, i=100500, s=15000, c2=67

# Дизассемблируем и смотрим на то, как код и данные размещаются в памяти


```cpp
%%cpp layout_example.c
%run arm-linux-gnueabi-gcc -marm layout_example.c -c -o layout_example.o

__asm__ (R"(
.global cut_struct
cut_struct:
    push {r4, r5} // notice that we decrease sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - obj, r1 - c, r2 - i, r3 - s, r4 - c2
    ldrb r5, [r0, #0]
    strb r5, [r1]
    ldr r5, [r0, #1]
    str r5, [r2]
    ldrh r5, [r0, #5]
    strh r5, [r3]
    ldrb r5, [r0, #7]
    strb r5, [r4]
    pop {r4, r5}
    push {r1-r12} // still one instruction
    pop {r1-r12}
    bx  lr
  
s1_ptr:
    .word s1
s2_ptr:
    .word s2
s1:
    .ascii "%d\n\0" // 4 bytes
s2:
    .ascii "%d%d\0" // 5 bytes
d1:
    .word 1234 // no padding
)");
```


Run: `arm-linux-gnueabi-gcc -marm layout_example.c -c -o layout_example.o`



```python
!arm-linux-gnueabi-objdump -D layout_example.o 2>&1 | grep cut_struct\> -A 35

```

    00000000 <cut_struct>:
       0:	e92d0030 	push	{r4, r5}
       4:	e59d4008 	ldr	r4, [sp, #8]
       8:	e5d05000 	ldrb	r5, [r0]
       c:	e5c15000 	strb	r5, [r1]
      10:	e5905001 	ldr	r5, [r0, #1]
      14:	e5825000 	str	r5, [r2]
      18:	e1d050b5 	ldrh	r5, [r0, #5]
      1c:	e1c350b0 	strh	r5, [r3]
      20:	e5d05007 	ldrb	r5, [r0, #7]
      24:	e5c45000 	strb	r5, [r4]
      28:	e8bd0030 	pop	{r4, r5}
      2c:	e92d1ffe 	push	{r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
      30:	e8bd1ffe 	pop	{r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip}
      34:	e12fff1e 	bx	lr
    
    00000038 <s1_ptr>:
      38:	00000040 	andeq	r0, r0, r0, asr #32
    
    0000003c <s2_ptr>:
      3c:	00000044 	andeq	r0, r0, r4, asr #32
    
    00000040 <s1>:
      40:	000a6425 	andeq	r6, sl, r5, lsr #8
    
    00000044 <s2>:
      44:	64256425 	strtvs	r6, [r5], #-1061	; 0xfffffbdb
    	...
    
    00000049 <d1>:
      49:	000004d2 	ldrdeq	r0, [r0], -r2
      4d:	Address 0x000000000000004d is out of bounds.
    
    
    Disassembly of section .comment:
    



```python

```

Можно сравнить с дизассемблером


```cpp
%%cpp cut_struct_disasm.c
%run arm-linux-gnueabi-gcc -marm cut_struct_disasm.c -O2 -S -o cut_struct_disasm.s
%run cat cut_struct_disasm.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

struct Obj {
    char c;
    int i;
    short s;
    char c2;
} __attribute__((packed));

int cut_struct(struct Obj* obj, char* c, int* i, short* s, char* c2) {
    *c = obj->c;
    *i = obj->i;
    *s = obj->s;
    *c2 = obj->c2;
}
```


Run: `arm-linux-gnueabi-gcc -marm cut_struct_disasm.c -O2 -S -o cut_struct_disasm.s`



Run: `cat cut_struct_disasm.s | grep -v "^\\s*\\." | grep -v "^\\s*@"`


    cut_struct:
    	mov	ip, r0
    	push	{r4, lr}
    	ldrb	r4, [ip]	@ zero_extendqisi2
    	mov	r0, lr
    	ldr	lr, [sp, #8]
    	strb	r4, [r1]
    	ldr	r1, [ip, #1]	@ unaligned
    	str	r1, [r2]
    	ldrsh	r2, [ip, #5]	@ unaligned
    	strh	r2, [r3]	@ movhi
    	ldrb	r3, [ip, #7]	@ zero_extendqisi2
    	strb	r3, [lr]
    	pop	{r4, pc}


# Размещение структур в памяти

Не все всегда расположено очевидным образом: для более эффективного выполнения программы бывает выгодно выровненное расположение объектов в памяти, например считывать ui64 из памяти выгоднее, если адрес делится на 8.

Примерные правила:
* выравнивание (то, на что адрес должен делиться) равно размеру для простых арифметических типов (указатели тоже здесь)
* в union берется максимум для выравнивания (и максимум из размеров округенный вверх, чтобы делиться на выравнивание)
* в struct члены располагаются в том порядке, в котором указаны. Выравнивание структуры - максимум выравниваний. Каждый член располагается так, чтобы удовлетворять собственному выравниванию. Итоговый размер структуры делится на выравнивание структуры. С учетом этого размер струкуры минимизируется.

Для экспериментов можно использовать `sizeof()` и `_Alignof()`, чтобы получить размер и выравнивание.


```cpp
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./structs_in_memory.exe

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define print_int(x) printf(#x " = %d\n", (int)x)

#define print_offset(type, field) {\
    type o; \
    printf("Shift of ." #field " in " #type ": %d\n", (int)((void*)&o.field - (void*)&o)); \
}

int main() {
    print_int(sizeof(char));
    print_int(_Alignof(char));
    print_int(sizeof(short));
    print_int(_Alignof(short));
    print_int(sizeof(int));
    print_int(_Alignof(int));
    print_int(sizeof(long long));
    print_int(_Alignof(long long));
    print_int(sizeof(double));
    print_int(_Alignof(double));

    typedef struct { // максимальное выравнивание у инта, значит выравнивание структуры 4
        char c;      // 0 байт
        int i;       // 4-7 байты
        char c2;     // 8 байт
    } Obj1_t;        // 9-11 - padding байты, чтобы размер делился на выравнивание
    print_int(sizeof(Obj1_t));
    print_int(_Alignof(Obj1_t));
    print_offset(Obj1_t, c);
    print_offset(Obj1_t, i);
    print_offset(Obj1_t, c2);
    
    typedef struct { // тут все правила про выравнивание не применимы, так как указан аттрибут упаковки
        char c;
        int i;
        char c2;
    } __attribute__((packed)) Obj2_t;
    print_int(sizeof(Obj2_t));
    print_int(_Alignof(Obj2_t));
    print_offset(Obj2_t, c);
    print_offset(Obj2_t, i);
    print_offset(Obj2_t, c2);
    
    typedef struct {  // максимальное выравнивание члена - 8, так что и у всей структуры такое же
        char c8;      // 0 байт 
        uint64_t u64; // 8-15 байты
    } Obj3_t;         // всего 16 байт, выравнивание 8
    print_int(sizeof(Obj3_t));
    print_int(_Alignof(Obj3_t));
    print_offset(Obj3_t, u64);
    print_offset(Obj3_t, c8);
    
    typedef struct {
        char c8;
        char c8_1;
        char c8_2;
    } Obj4_t;
    print_int(sizeof(Obj4_t));
    print_int(_Alignof(Obj4_t));
    
    typedef struct {     // тут пример двух структур равного размера, но с разным выравниванием
        long long a;
    } ObjS8A8;
    print_int(sizeof(ObjS8A8));
    print_int(_Alignof(ObjS8A8));
    typedef struct {
        int a;
        int b;
    } ObjS8A4;
    print_int(sizeof(ObjS8A4));
    print_int(_Alignof(ObjS8A4));
    
    typedef struct {    // и вот тут разное выравнивание ObjS8A8 и ObjS8A4 себя покажет
        ObjS8A8 o;
        char c;
    } Obj5_t;
    print_int(sizeof(Obj5_t)); // обратите внимание на разницу с Obj6_t!
    print_int(_Alignof(Obj5_t));
    
    typedef struct {
        ObjS8A4 o;
        char c;
    } Obj6_t;
    print_int(sizeof(Obj6_t));
    print_int(_Alignof(Obj6_t));
    
    typedef union {
        unsigned long long u;
        int i[3];
    } Obj7_t;
    print_int(sizeof(Obj7_t));
    print_int(_Alignof(Obj7_t));
    
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./structs_in_memory.exe`


    sizeof(char) = 1
    _Alignof(char) = 1
    _Alignof(_Atomic short) = 2
    sizeof(short) = 2
    _Alignof(short) = 2
    sizeof(int) = 4
    _Alignof(int) = 4
    sizeof(long long) = 8
    _Alignof(long long) = 8
    sizeof(double) = 8
    _Alignof(double) = 8
    sizeof(Obj1_t) = 12
    _Alignof(Obj1_t) = 4
    Shift of .c in Obj1_t: 0
    Shift of .i in Obj1_t: 4
    Shift of .c2 in Obj1_t: 8
    sizeof(Obj2_t) = 6
    _Alignof(Obj2_t) = 1
    Shift of .c in Obj2_t: 0
    Shift of .i in Obj2_t: 1
    Shift of .c2 in Obj2_t: 5
    sizeof(Obj3_t) = 16
    _Alignof(Obj3_t) = 8
    Shift of .u64 in Obj3_t: 8
    Shift of .c8 in Obj3_t: 0
    sizeof(Obj4_t) = 3
    _Alignof(Obj4_t) = 1
    sizeof(ObjS8A8) = 8
    _Alignof(ObjS8A8) = 8
    sizeof(ObjS8A4) = 8
    _Alignof(ObjS8A4) = 4
    sizeof(Obj5_t) = 16
    _Alignof(Obj5_t) = 8
    sizeof(Obj6_t) = 12
    _Alignof(Obj6_t) = 4
    sizeof(Obj7_t) = 16
    _Alignof(Obj7_t) = 8


Попробовать _Atomic + `__packed__`

# Вызов функций


```cpp
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

#include <stdio.h>

int print_a(char a) {
    fputc(a, stdout);
    return 1;
}
```


Run: `arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s`



Run: `cat call.s | grep -v "^\\s*\\." | grep -v "^\\s*@"`


    print_a:
    	movw	r3, #:lower16:stdout
    	push	{r4, lr}
    	movt	r3, #:upper16:stdout
    	ldr	r1, [r3]
    	bl	fputc
    	mov	r0, #1
    	pop	{r4, pc}


https://stackoverflow.com/questions/17214962/what-is-the-difference-between-label-equals-sign-and-label-brackets-in-ar

https://stackoverflow.com/questions/14046686/why-use-ldr-over-mov-or-vice-versa-in-arm-assembly


```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int print_a(char a);
__asm__(R"(
print_a:
    push {lr}
    ldr r3, =stdout
    ldr r1, [r3]
    bl fputc
    mov r0, #1
    pop {pc}
)");

int main() {
    print_a('?');
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    ?

### Форматированный вывод


```cpp
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s

#include <stdio.h>

int print_a(int a) {
    fprintf(stdout, "%d\n", a);
    return 42;
}
```


Run: `arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s`



Run: `cat call.s`


    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 2
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"call.c"
    	.text
    	.align	2
    	.global	print_a
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	print_a, %function
    print_a:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 0, uses_anonymous_args = 0
    	movw	r3, #:lower16:stdout
    	movw	r1, #:lower16:.LC0
    	movt	r3, #:upper16:stdout
    	mov	r2, r0
    	push	{r4, lr}
    	movt	r1, #:upper16:.LC0
    	ldr	r0, [r3]
    	bl	fprintf
    	mov	r0, #42
    	pop	{r4, pc}
    	.size	print_a, .-print_a
    	.section	.rodata.str1.4,"aMS",%progbits,1
    	.align	2
    .LC0:
    	.ascii	"%d\012\000"
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int print_a(int a);
__asm__(R"(
    .text
    .global print_a
print_a:
    mov r2, r0
        
    ldr r0, =stdout
    ldr r0, [r0]
    ldr r1, =.format_string
    
    push {lr}
    bl fprintf
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d\n"
    .ascii "\0"
)");

int main() {
    print_a(100500);
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    100500



```python

```

### Форматированное чтение


```cpp
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s

#include <stdio.h>

int scan_a(int* a) {
    fscanf(stdin, "%d", a);
    return 42;
}
```


Run: `arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s`



Run: `cat call.s`


    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 2
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"call.c"
    	.text
    	.align	2
    	.global	scan_a
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	scan_a, %function
    scan_a:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 0, uses_anonymous_args = 0
    	movw	r3, #:lower16:stdin
    	movw	r1, #:lower16:.LC0
    	movt	r3, #:upper16:stdin
    	mov	r2, r0
    	push	{r4, lr}
    	movt	r1, #:upper16:.LC0
    	ldr	r0, [r3]
    	bl	__isoc99_fscanf
    	mov	r0, #42
    	pop	{r4, pc}
    	.size	scan_a, .-scan_a
    	.section	.rodata.str1.4,"aMS",%progbits,1
    	.align	2
    .LC0:
    	.ascii	"%d\000"
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int scan_a(int* a);
__asm__(R"(
    .text
    .global scan_a
scan_a:
    mov r2, r0
    mov r3, r0
    ldr r0, =stdin
    ldr r0, [r0]
    ldr r1, =.format_string
    push {lr}
    push {r2}
    bl __isoc99_fscanf
    pop {r2}
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d %d %d\0"
)");

int main() {
    int a = 100500;
    scan_a(&a);
    printf("a = %d\n", a);
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    a = 125



```python
!arm-linux-gnueabi-objdump -D test_call.exe 2>&1 | grep 0001047c -A 20
```

    0001047c <scan_a>:
       1047c:	e1a02000 	mov	r2, r0
       10480:	e1a03000 	mov	r3, r0
       10484:	e59f0028 	ldr	r0, [pc, #40]	; 104b4 <.format_string+0xc>
       10488:	e5900000 	ldr	r0, [r0]
       1048c:	e59f1024 	ldr	r1, [pc, #36]	; 104b8 <.format_string+0x10>
       10490:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
       10494:	e52d2004 	push	{r2}		; (str r2, [sp, #-4]!)
       10498:	ebffffad 	bl	10354 <__isoc99_fscanf@plt>
       1049c:	e49d2004 	pop	{r2}		; (ldr r2, [sp], #4)
       104a0:	e3a0002a 	mov	r0, #42	; 0x2a
       104a4:	e49df004 	pop	{pc}		; (ldr pc, [sp], #4)
    
    000104a8 <.format_string>:
       104a8:	25206425 	strcs	r6, [r0, #-1061]!	; 0xfffffbdb
       104ac:	64252064 	strtvs	r2, [r5], #-100	; 0xffffff9c
       104b0:	00000000 	andeq	r0, r0, r0
       104b4:	00021030 	andeq	r1, r2, r0, lsr r0
       104b8:	000104a8 	andeq	r0, r1, r8, lsr #9
    
    000104bc <__libc_csu_init>:



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int ret_eof();
__asm__(R"(
#include <stdio.h>
    .text
    .global ret_eof
ret_eof:
    mov r0, =EOF
    bx lr
)");

int main() {
    printf("%d\n", ret_eof());
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`


    /tmp/ccLQuae7.s: Assembler messages:
    /tmp/ccLQuae7.s:19: Error: immediate expression requires a # prefix -- `mov r0,=EOF'



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    a = 125


# Решение одной домашней задачи


```python
%%asm sol.S
%run arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe

.global main
main:
    push {lr}

cin_symb:
    ldr r0, =stdin
    ldr r0, [r0]
    bl fgetc

    cmp r0, #0
    blt out

    cmp r0, #'0'
    ble cin_symb

    cmp r0, #'9'
    bge cin_symb

    ldr r1, =stdout
    ldr r1, [r1]
    bl fputc
    b cin_symb
out:
    pop {pc}
```


Run: `arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe`



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe`


    123124125


```python

```


```python

```
