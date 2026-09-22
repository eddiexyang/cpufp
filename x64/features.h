#ifndef CPUFP_X64_FEATURES_H
#define CPUFP_X64_FEATURES_H

#include <stdint.h>
#include <string.h>

#define CPUFP_BIT(n) (UINT32_C(1) << (n))

enum cpufp_feature {
    CPUFP_SSE = CPUFP_BIT(0),
    CPUFP_SSE2 = CPUFP_BIT(1),
    CPUFP_AVX = CPUFP_BIT(2),
    CPUFP_FMA = CPUFP_BIT(3),
    CPUFP_AVX2 = CPUFP_BIT(4),
    CPUFP_AVX512F = CPUFP_BIT(5),
    CPUFP_AVX512VL = CPUFP_BIT(6),
    CPUFP_AVX512_VNNI = CPUFP_BIT(7),
    CPUFP_AVX512_BF16 = CPUFP_BIT(8),
    CPUFP_AVX512_FP16 = CPUFP_BIT(9),
    CPUFP_AVX_VNNI = CPUFP_BIT(10),
    CPUFP_AVX_VNNI_INT8 = CPUFP_BIT(11),
    CPUFP_AVX_VNNI_INT16 = CPUFP_BIT(12),
    CPUFP_AMX_INT8 = CPUFP_BIT(13),
    CPUFP_AMX_BF16 = CPUFP_BIT(14),
    CPUFP_AMX_FP16 = CPUFP_BIT(15),
};

struct cpufp_cpuid_regs { uint32_t eax, ebx, ecx, edx; };
struct cpufp_cpu_state {
    struct cpufp_cpuid_regs leaf1, leaf7, leaf7_1;
    uint64_t xcr0;
    int amx_permission;
};

/* Kept independent of the host so OS-disabled states can be regression-tested. */
static inline uint32_t cpufp_decode_features(const struct cpufp_cpu_state *s)
{
    uint32_t f = 0;
    const uint32_t xsave = CPUFP_BIT(26) | CPUFP_BIT(27);
    const int os_xsave = (s->leaf1.ecx & xsave) == xsave;
    const int avx = os_xsave && (s->leaf1.ecx & CPUFP_BIT(28)) &&
                    (s->xcr0 & UINT64_C(0x6)) == UINT64_C(0x6);
    const int avx512 = avx && (s->leaf7.ebx & CPUFP_BIT(16)) &&
                    (s->xcr0 & UINT64_C(0xe6)) == UINT64_C(0xe6);
    if (s->leaf1.edx & CPUFP_BIT(25)) f |= CPUFP_SSE;
    if (s->leaf1.edx & CPUFP_BIT(26)) f |= CPUFP_SSE2;
    if (avx) {
        f |= CPUFP_AVX;
        if (s->leaf1.ecx & CPUFP_BIT(12)) f |= CPUFP_FMA;
        if (s->leaf7.ebx & CPUFP_BIT(5)) {
            f |= CPUFP_AVX2;
            /* The 256-bit kernels also use AVX2 VPXOR. */
            if (s->leaf7_1.eax & CPUFP_BIT(4)) f |= CPUFP_AVX_VNNI;
            if (s->leaf7_1.edx & CPUFP_BIT(4)) f |= CPUFP_AVX_VNNI_INT8;
            if (s->leaf7_1.edx & CPUFP_BIT(10)) f |= CPUFP_AVX_VNNI_INT16;
        }
    }
    if (avx512) {
        f |= CPUFP_AVX512F;
        if (s->leaf7.ebx & CPUFP_BIT(31)) f |= CPUFP_AVX512VL;
        if (s->leaf7.ecx & CPUFP_BIT(11)) f |= CPUFP_AVX512_VNNI;
        if (s->leaf7_1.eax & CPUFP_BIT(5)) f |= CPUFP_AVX512_BF16;
        if (s->leaf7.edx & CPUFP_BIT(23)) f |= CPUFP_AVX512_FP16;
    }
    if (os_xsave && s->amx_permission &&
        (s->xcr0 & UINT64_C(0x60000)) == UINT64_C(0x60000) &&
        (s->leaf7.edx & CPUFP_BIT(24))) {
        if (s->leaf7.edx & CPUFP_BIT(25)) f |= CPUFP_AMX_INT8;
        if (s->leaf7.edx & CPUFP_BIT(22)) f |= CPUFP_AMX_BF16;
        if (s->leaf7_1.eax & CPUFP_BIT(21)) f |= CPUFP_AMX_FP16;
    }
    return f;
}

static inline int cpufp_feature_available(uint32_t features,
                                        const char *isa, const char *width)
{
    uint32_t required = 0;
    if (strcmp(isa, "SSE") == 0) required = CPUFP_SSE;
    if (strcmp(isa, "SSE2") == 0) required = CPUFP_SSE2;
    if (strcmp(isa, "AVX") == 0) required = CPUFP_AVX;
    if (strcmp(isa, "FMA") == 0) required = CPUFP_FMA;
    if (strcmp(isa, "AVX512F") == 0) required = CPUFP_AVX512F;
    if (strcmp(isa, "AVX512_VNNI") == 0) required = CPUFP_AVX512_VNNI;
    if (strcmp(isa, "AVX512_BF16") == 0) required = CPUFP_AVX512_BF16;
    if (strcmp(isa, "AVX512_FP16") == 0) required = CPUFP_AVX512_FP16;
    if (strcmp(isa, "AVX_VNNI") == 0) required = CPUFP_AVX_VNNI;
    if (strcmp(isa, "AVX_VNNI_INT8") == 0) required = CPUFP_AVX_VNNI_INT8;
    if (strcmp(isa, "AVX_VNNI_INT16") == 0) required = CPUFP_AVX_VNNI_INT16;
    if (strcmp(isa, "AMX_INT8") == 0) required = CPUFP_AMX_INT8;
    if (strcmp(isa, "AMX_BF16") == 0) required = CPUFP_AMX_BF16;
    if (strcmp(isa, "AMX_FP16") == 0) required = CPUFP_AMX_FP16;
    if (!required) return 0;
    if (strncmp(isa, "AVX512", 6) == 0 && strcmp(width, "512b") != 0) {
        required |= CPUFP_AVX512VL;
        if (strcmp(width, "256b") == 0) required |= CPUFP_AVX2;
    }
    return (features & required) == required;
}

#ifdef __cplusplus
extern "C" {
#endif
uint32_t cpufp_detect_features(int request_amx);
#ifdef __cplusplus
}
#endif
#endif
