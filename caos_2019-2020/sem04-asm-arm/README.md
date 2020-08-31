



```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/Downloads/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/"

```

#### Makefile в котором будет компиляция и запуски всех примеров


```python
%%makefile 

GCC=arm-linux-gnueabi-gcc -marm
RUN=qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi

hello:
    ${GCC} hello.c -o hello.exe
    ${GCC} hello.c -S -o hello.S

hello_run: hello
    ${RUN} ./hello.exe   
    
lib_sum:
    ${GCC} lib_sum.c -c
    ${GCC} lib_sum.c -S -o lib_sum.S
    ${GCC} lib_sum.c -O0 -S -o lib_sum_o0.S
    ${GCC} lib_sum.c -O3 -S -o lib_sum_o3.S
    
my_lib:
    ${GCC} -g my_lib_sum.S -c
    
my_lib_example: my_lib
    ${GCC} -std=c99 -g my_lib_sum.o my_lib_example.c -o my_lib_example.exe
    
my_lib_example_run: my_lib_example
    ${RUN} ./my_lib_example.exe 
    
my_lib_example_run_gdb: my_lib_example
    ${RUN} -g 1234 ./my_lib_example.exe 
    # Подключаться с помощью gdb можно так:
    # gdb-multiarch -q --nh   -ex 'set architecture arm'   -ex 'set sysroot ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi'   -ex 'file ./my_lib_example.exe'   -ex 'target remote localhost:1234'   -ex 'break main'   -ex continue   -ex 'layout split'
    
asm_inline_example:
    ${GCC} asm_inline_example.c -o asm_inline_example.exe
    
asm_inline_example_run: asm_inline_example
    ${RUN} ./asm_inline_example.exe
    
```


```cpp
%%cpp hello.c
%run make hello_run

// Скомпилируем под arm и запустим hello_world 

#include <stdio.h>

int main() {
    printf("hello world!\n");
    return 0;
}

```


Run: `make hello_run`


    arm-linux-gnueabi-gcc -marm hello.c -o hello.exe
    arm-linux-gnueabi-gcc -marm hello.c -S -o hello.S
    qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./hello.exe   
    hello world!



```python
!cat hello.S
```

    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 6
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"hello.c"
    	.text
    	.section	.rodata
    	.align	2
    .LC0:
    	.ascii	"hello world!\000"
    	.text
    	.align	2
    	.global	main
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	main, %function
    main:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 1, uses_anonymous_args = 0
    	push	{fp, lr}
    	add	fp, sp, #4
    	movw	r0, #:lower16:.LC0
    	movt	r0, #:upper16:.LC0
    	bl	puts
    	mov	r3, #0
    	mov	r0, r3
    	pop	{fp, pc}
    	.size	main, .-main
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits


#### Напишем и скомпилируем до состояния arm'ного ассемблера простую функцию


```cpp
%%cpp lib_sum.c
%run make lib_sum

int sum(int a, int b) {
    return a + b;
}
```


Run: `make lib_sum`


    arm-linux-gnueabi-gcc -marm lib_sum.c -c
    arm-linux-gnueabi-gcc -marm lib_sum.c -S -o lib_sum.S
    arm-linux-gnueabi-gcc -marm lib_sum.c -O0 -S -o lib_sum_o0.S
    arm-linux-gnueabi-gcc -marm lib_sum.c -O3 -S -o lib_sum_o3.S



```python
# Здесь можно посмотреть, что получается при O0 и O3
!cat lib_sum_o0.S
```

    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 6
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"lib_sum.c"
    	.text
    	.align	2
    	.global	sum
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	sum, %function
    sum:
    	@ args = 0, pretend = 0, frame = 8
    	@ frame_needed = 1, uses_anonymous_args = 0
    	@ link register save eliminated.
    	str	fp, [sp, #-4]!
    	add	fp, sp, #0
    	sub	sp, sp, #12
    	str	r0, [fp, #-8]
    	str	r1, [fp, #-12]
    	ldr	r2, [fp, #-8]
    	ldr	r3, [fp, #-12]
    	add	r3, r2, r3
    	mov	r0, r3
    	add	sp, fp, #0
    	@ sp needed
    	ldr	fp, [sp], #4
    	bx	lr
    	.size	sum, .-sum
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits


#### А теперь самостоятельно напишем ту же функцию. Как видим кода стало меньше :) И потом вызовем ее из кода на С


```python
%%asm my_lib_sum.S
.text
.global sum
sum:
    add r0, r0, r1
    bx  lr
```


```cpp
%%cpp my_lib_example.c
%run make my_lib_example_run

#include <stdio.h>

int sum(int, int);

int main() {
    printf("40 + 2 = %d\n", sum(40, 2));
    return 0;
}
```


Run: `make my_lib_example_run`


    arm-linux-gnueabi-gcc -marm -g my_lib_sum.S -c
    arm-linux-gnueabi-gcc -marm -std=c99 -g my_lib_sum.o my_lib_example.c -o my_lib_example.exe
    qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./my_lib_example.exe 
    40 + 2 = 42



```python

```


```cpp
%%cpp asm_inline_example.c
%run make asm_inline_example_run

#include <stdio.h>

int sum(int a, int b) {
    return a + b;
}


int sum2(int, int);
__asm__ (R"(
.global sum2
sum2:
    add r0, r0, r1
    bx  lr
)");


int main() {
    printf("40 + 2 = %d\n", sum(40, 2));
    printf("40 + 2 = %d\n", sum2(40, 2));
    return 0;
}
```


Run: `make asm_inline_example_run`


    arm-linux-gnueabi-gcc -marm asm_inline_example.c -o asm_inline_example.exe
    qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./asm_inline_example.exe
    40 + 2 = 42
    40 + 2 = 42



```python

```

# Полезные фишки

`cmp r0, #'9'` - так можно писать char-константы

`push {r4, r5, r6, r7, r8, lr}` <-> `push {r4-r8, lr}`


```python

```
