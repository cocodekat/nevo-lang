.data
str_0:
    .asciz "%d"
.text
.text
.global _entry
_entry:
.data
str_1:
    .asciz "Hello World"
.text
    // print string literal
    ldr x16, =0x2000004
    mov x0, 1
    adrp x1, str_1@PAGE
    add x1, x1, str_1@PAGEOFF
    mov x2, 11
    svc 0
    mov w0, #5
    adrp x9, var_0@PAGE
    str  w0, [x9, var_0@PAGEOFF]
    adrp x9, var_0@PAGE
    ldr  w0, [x9, var_0@PAGEOFF]
    ldr x16, =0x2000001   // exit syscall
    mov x0, 0
    svc 0
.data
var_0: .word 0
