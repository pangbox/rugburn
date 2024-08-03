.386
.model flat

.code

public __fltused
__fltused equ 9876h

public __ldused
__ldused equ 9876h

public __except_list
__except_list equ 0

public __aullshr
__aullshr PROC
    cmp cl, 40h
    jae retzero
    cmp cl, 20h
    jae more32
    shrd eax, edx, cl
    shr edx, cl
    ret
more32:
    mov eax, edx
    xor edx, edx
    and cl, 1Fh
    shr eax, cl
    ret
retzero:
    xor eax, eax
    xor edx, edx
    ret
__aullshr ENDP

END
