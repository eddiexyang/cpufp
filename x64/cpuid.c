#include "features.h"
#include <stdio.h>
#if defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
#endif

static struct cpufp_cpuid_regs cpuid_exec(uint32_t leaf, uint32_t subleaf)
{
    struct cpufp_cpuid_regs r;
    __asm__ volatile ("cpuid" : "=a"(r.eax), "=b"(r.ebx),
                      "=c"(r.ecx), "=d"(r.edx) : "a"(leaf), "c"(subleaf));
    return r;
}

uint32_t cpufp_detect_features(int request_amx)
{
    struct cpufp_cpu_state s = {0};
    uint32_t max_leaf = cpuid_exec(0, 0).eax;
    if (max_leaf >= 1) s.leaf1 = cpuid_exec(1, 0);
    if (max_leaf >= 7) {
        s.leaf7 = cpuid_exec(7, 0);
        if (s.leaf7.eax >= 1) s.leaf7_1 = cpuid_exec(7, 1);
    }
    /* XGETBV itself faults unless both XSAVE and OSXSAVE are advertised. */
    if ((s.leaf1.ecx & (CPUFP_BIT(26) | CPUFP_BIT(27))) ==
                      (CPUFP_BIT(26) | CPUFP_BIT(27))) {
        uint32_t lo, hi;
        __asm__ volatile ("xgetbv" : "=a"(lo), "=d"(hi) : "c"(0));
        s.xcr0 = ((uint64_t)hi << 32) | lo;
    }
#if defined(__linux__) && defined(SYS_arch_prctl)
    if ((s.leaf7.edx & CPUFP_BIT(24)) &&
        (s.xcr0 & UINT64_C(0x60000)) == UINT64_C(0x60000)) {
        /* Linux permission is process-wide, inherited by newly created workers.
           It is reset by exec, so the benchmark must request it again. */
        unsigned long permitted = 0;
        if (!request_amx || syscall(SYS_arch_prctl, 0x1023, 18) == 0) {
            if (syscall(SYS_arch_prctl, 0x1022, &permitted) == 0)
                s.amx_permission = (permitted & 0x60000UL) == 0x60000UL;
        } else {
            fprintf(stderr, "Warning: AMX permission denied; skipping AMX.\n");
        }
    }
#else
    (void)request_amx;
#endif
    return cpufp_decode_features(&s);
}

#ifndef CPUFP_CPUID_NO_MAIN
int main(void)
{
    uint32_t features = cpufp_detect_features(1);
    if (features & CPUFP_SSE) puts("_SSE_");
    if (features & CPUFP_SSE2) puts("_SSE2_");
    if (features & CPUFP_AVX) puts("_AVX_");
    if (features & CPUFP_FMA) puts("_FMA_");
    if (features & CPUFP_AVX512F) puts("_AVX512F_");
    if (features & CPUFP_AVX512_VNNI) puts("_AVX512_VNNI_");
    if (features & CPUFP_AVX512_BF16) puts("_AVX512_BF16_");
    if (features & CPUFP_AVX512_FP16) puts("_AVX512_FP16_");
    if (features & CPUFP_AVX_VNNI) puts("_AVX_VNNI_");
    if (features & CPUFP_AVX_VNNI_INT8) puts("_AVX_VNNI_INT8_");
    if (features & CPUFP_AVX_VNNI_INT16) puts("_AVX_VNNI_INT16_");
    if (features & CPUFP_AMX_INT8) puts("_AMX_INT8_");
    if (features & CPUFP_AMX_BF16) puts("_AMX_BF16_");
    if (features & CPUFP_AMX_FP16) puts("_AMX_FP16_");
    return 0;
}
#endif
