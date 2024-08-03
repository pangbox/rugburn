.intel_syntax noprefix

.global __fltused
.set __fltused, 0x9876

.global __ldused
.set __ldused, 0x9876

.global __except_list
.set __except_list, 0

.section .text

.global __aullshr
__aullshr:
    cmp cl, 0x40
    jae retzero
    cmp cl, 0x20
    jae more32
    shrd eax, edx, cl
    shr edx, cl
    ret
more32:
    mov eax, edx
    xor edx, edx
    and cl, 0x1f
    shr eax, cl
    ret
retzero:
    xor eax, eax
    xor edx, edx
    ret

.end
