.text
.data
str_newline: .asciz "\n"
.text
.text
.global _main
_main:
    mov w0, #5
    adrp x9, a@PAGE
    str  w0, [x9, a@PAGEOFF]
    adrp x9, a@PAGE
    ldr w0, [x9, a@PAGEOFF]
    mov w1, #5
    cmp w0, w1
    b.ne if_else_0
    adrp x9, a@PAGE
    ldr w0, [x9, a@PAGEOFF]
    mov w1, #1
    add w0, w0, w1
    adrp x9, _loop_counter_0@PAGE
    str w0, [x9, _loop_counter_0@PAGEOFF]
_loop_0:
    adrp x9, _loop_counter_0@PAGE
    ldr w0, [x9, _loop_counter_0@PAGEOFF]
    cbz w0, _loop_end_0
    adrp x9, a@PAGE
    ldr w0, [x9, a@PAGEOFF]
    mov w1, #5
    cmp w0, w1
    b.ne if_else_1
    // print string literal
    ldr x16, =0x2000004
    mov x0, 1
    adrp x1, str_1@PAGE
    add x1, x1, str_1@PAGEOFF
    mov x2, 4
    svc 0
    adrp x9, _loop_counter_0@PAGE
    ldr w0, [x9, _loop_counter_0@PAGEOFF]
    sub w0, w0, #1
    str w0, [x9, _loop_counter_0@PAGEOFF]
    b _loop_0
_loop_end_0:
if_else_1:
if_else_0:
    ldr x16, =0x2000001   // exit syscall
    mov x0, 0
    svc 0
.data
.align 2
a: .word 0
.align 2
_loop_counter_0: .word 0
.data
str_0:
    .asciz "%d"
str_1:
    .asciz "hi\n"
