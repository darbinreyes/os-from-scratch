#if 0
int my_function(void) {
    return 0xbaba;
}

/*
i386-elf-gcc -O0 -Wall -Wextra -ffreestanding -c c0.c -o c0.o
i386-elf-ld -O0 -o c0.bin -Ttext 0x0 --oformat binary c0.o
ndisasm -b 32 c0.bin
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
                                           ; [        ]                      <- EBP

                                           ; [ . . .  ]

                                           ; [esp     ] Caller EIP    <- ESP
00000000  55                push ebp
                                           ; [esp + 4 ] Caller EIP
                                           ; [esp     ] Caller EBP    <- ESP
00000001  89E5              mov ebp,esp
                                           ; [esp + 4 ] Caller EIP
                                           ; [esp     ] Caller EBP    <- ESP <-EBP
00000003  B8BABA0000        mov eax,0xbaba
00000008  5D                pop ebp
                                           ; [esp + 4 ] Caller EIP    <- ESP
00000009  C3                ret
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
*/
#endif

#if 0
int my_function(void) {
    int my_var = 0xbaba;
    return my_var;
}

/*
i386-elf-gcc -O0 -Wall -Wextra -ffreestanding -c c0.c -o c0.o
i386-elf-ld -O0 -o c0.bin -Ttext 0x0 --oformat binary c0.o
ndisasm -b 32 c0.bin
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
00000000  55                push ebp
00000001  89E5              mov ebp,esp
00000003  83EC10            sub esp,byte +0x10
00000006  C745FCBABA0000    mov dword [ebp-0x4],0xbaba
0000000D  8B45FC            mov eax,[ebp-0x4]
00000010  C9                leave                       ; leave:
                                                        ;       mov esp, ebp
                                                        ;       pop ebp
00000011  C3                ret
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
*/
#endif

#if 0
int callee_function(int my_arg);

void caller_function(void) {
    callee_function(0xdede);
}

int callee_function(int my_arg) {
    return my_arg;
}

/*
i386-elf-gcc -O0 -Wall -Wextra -ffreestanding -c c0.c -o c0.o
i386-elf-ld -O0 -o c0.bin -Ttext 0x0 --oformat binary c0.o
ndisasm -b 32 c0.bin
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
00000000  55                push ebp
00000001  89E5              mov ebp,esp
00000003  83EC08            sub esp,byte +0x8
00000006  83EC0C            sub esp,byte +0xc
00000009  68DEDE0000        push dword 0xdede
0000000E  E806000000        call 0x19
00000013  83C410            add esp,byte +0x10
00000016  90                nop                ; @ref Intel SDM Vol.2.
                                               ; NOP — No Operation
                                               ; The one-byte NOP instruction is an alias mnemonic for the XCHG (E)AX, (E)AX instruction.
                                               ; This instruction performs no operation. It is a one-byte or multi-byte NOP that takes up space in the instruction stream but does not impact machine context, except for the EIP register.
                                               ; Guess: Probably inserted for alignment reasons.
00000017  C9                leave
00000018  C3                ret
00000019  55                push ebp
0000001A  89E5              mov ebp,esp
0000001C  8B4508            mov eax,[ebp+0x8]
0000001F  5D                pop ebp
00000020  C3                ret
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
*/
#endif


/*

The disassembly of the following function does not match Nick Blundell's text.
Why?

* Nick Blundell may have made a typo in the commands he specifies for disassembling. He is also working on a Linux system with Linux build tools. Differences are to be expected.

* If you disassemble the object file with `i386-elf-objdump`, linking hasn't happened yet, so addresses will not appear as they would in the final executable.

* The use of `ld --oformat binary` seems to throw off the disassembly. `ld --oformat elf32-i386` seems to produce more reasonable results when the disassembly looks strange.

* The safest option seems to be to use `ndisasm -b 32`. The extra instructions at the end can be ignored because:
"The only problem with disassembling machine code is that some of those bytes
may have been reserved as data but will show up as assembly instructions,
though in our simple C program we didn’t declare any data."

*/

#if 1
void my_function(void) {
    char my_string[] = "Hello"; // Nick Blundell wrote this `char *my_string = "Hello";` but he meant `char my_string[] = "Hello";`
}
/*
i386-elf-gcc -O0 -Wall -Wextra -ffreestanding -c c0.c -o c0.o
i386-elf-ld -O0 -o c0.bin -Ttext 0x0 --oformat binary c0.o
ndisasm -b 32 c0.bin
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
00000000  55                push ebp
00000001  89E5              mov ebp,esp
00000003  83EC10            sub esp,byte +0x10
00000006  C745FA48656C6C    mov dword [ebp-0x6],0x6c6c6548
0000000D  66C745FE6F00      mov word [ebp-0x2],0x6f
00000013  90                nop
00000014  C9                leave
00000015  C3                ret
- - - - - - - - - - - -- - - - - -- - - - - -- - - - - -
*/
#endif


