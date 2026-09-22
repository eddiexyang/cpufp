#include "symbols.h"
.text

.globl CPUFP_SYMBOL(amx_int8_mm_s32s8s8)
.globl CPUFP_SYMBOL(amx_int8_mm_s32s8u8)
.globl CPUFP_SYMBOL(amx_int8_mm_s32u8s8)
.globl CPUFP_SYMBOL(amx_int8_mm_s32u8u8)

CPUFP_SYMBOL(amx_int8_mm_s32s8s8):
    ldtilecfg (%rsi)
    tilezero %tmm0
    tilezero %tmm1
    tilezero %tmm2
    tilezero %tmm3
    tilezero %tmm4
    tilezero %tmm5
L.amx.int8.mm.s32s8s8:
    tdpbssd %tmm4, %tmm5, %tmm0
    tdpbssd %tmm4, %tmm5, %tmm1
    tdpbssd %tmm4, %tmm5, %tmm2
    tdpbssd %tmm4, %tmm5, %tmm3
    sub $0x1, %rdi
    jne L.amx.int8.mm.s32s8s8
    ret

CPUFP_SYMBOL(amx_int8_mm_s32s8u8):
    ldtilecfg (%rsi)
    tilezero %tmm0
    tilezero %tmm1
    tilezero %tmm2
    tilezero %tmm3
    tilezero %tmm4
    tilezero %tmm5
L.amx.int8.mm.s32s8u8:
    tdpbsud %tmm4, %tmm5, %tmm0
    tdpbsud %tmm4, %tmm5, %tmm1
    tdpbsud %tmm4, %tmm5, %tmm2
    tdpbsud %tmm4, %tmm5, %tmm3
    sub $0x1, %rdi
    jne L.amx.int8.mm.s32s8u8
    ret

CPUFP_SYMBOL(amx_int8_mm_s32u8s8):
    ldtilecfg (%rsi)
    tilezero %tmm0
    tilezero %tmm1
    tilezero %tmm2
    tilezero %tmm3
    tilezero %tmm4
    tilezero %tmm5
L.amx.int8.mm.s32u8s8:
    tdpbusd %tmm4, %tmm5, %tmm0
    tdpbusd %tmm4, %tmm5, %tmm1
    tdpbusd %tmm4, %tmm5, %tmm2
    tdpbusd %tmm4, %tmm5, %tmm3
    sub $0x1, %rdi
    jne L.amx.int8.mm.s32u8s8
    ret

CPUFP_SYMBOL(amx_int8_mm_s32u8u8):
    ldtilecfg (%rsi)
    tilezero %tmm0
    tilezero %tmm1
    tilezero %tmm2
    tilezero %tmm3
    tilezero %tmm4
    tilezero %tmm5
L.amx.int8.mm.s32u8u8:
    tdpbuud %tmm4, %tmm5, %tmm0
    tdpbuud %tmm4, %tmm5, %tmm1
    tdpbuud %tmm4, %tmm5, %tmm2
    tdpbuud %tmm4, %tmm5, %tmm3
    sub $0x1, %rdi
    jne L.amx.int8.mm.s32u8u8
    ret

