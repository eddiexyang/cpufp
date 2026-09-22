#include "../x64/features.h"
#include <assert.h>
#include <stdio.h>

static struct cpufp_cpu_state full_state(void)
{
    struct cpufp_cpu_state s = {0};
    s.leaf1.ecx = CPUFP_BIT(12) | CPUFP_BIT(26) | CPUFP_BIT(27) | CPUFP_BIT(28);
    s.leaf1.edx = CPUFP_BIT(25) | CPUFP_BIT(26);
    s.leaf7.ebx = CPUFP_BIT(5) | CPUFP_BIT(16) | CPUFP_BIT(31);
    s.leaf7.ecx = CPUFP_BIT(11);
    s.leaf7.edx = CPUFP_BIT(22) | CPUFP_BIT(23) | CPUFP_BIT(24) | CPUFP_BIT(25);
    s.leaf7_1.eax = CPUFP_BIT(4) | CPUFP_BIT(5) | CPUFP_BIT(21);
    s.leaf7_1.edx = CPUFP_BIT(4) | CPUFP_BIT(10);
    s.xcr0 = UINT64_C(0x600e7);
    s.amx_permission = 1;
    return s;
}

int main(void)
{
    struct cpufp_cpu_state s = {0};
    uint32_t f;
    const uint32_t baseline = CPUFP_SSE | CPUFP_SSE2;
    const uint32_t amx = CPUFP_AMX_INT8 | CPUFP_AMX_BF16 | CPUFP_AMX_FP16;
    assert(cpufp_decode_features(&s) == 0);
    s = full_state();
    assert(cpufp_decode_features(&s) == 0xffffU);

    s.leaf1.ecx &= ~CPUFP_BIT(27); /* OSXSAVE absent: never trust XCR0. */
    assert(cpufp_decode_features(&s) == baseline);
    s = full_state();
    s.leaf1.ecx &= ~CPUFP_BIT(26);
    assert(cpufp_decode_features(&s) == baseline);
    s = full_state();
    s.xcr0 = 1; /* OS saves only x87. */
    assert(cpufp_decode_features(&s) == baseline);
    s.xcr0 = 3; /* XMM without YMM. */
    assert(cpufp_decode_features(&s) == baseline);
    s.xcr0 = 5; /* YMM without XMM. */
    assert(cpufp_decode_features(&s) == baseline);
    s = full_state();
    s.leaf1.ecx &= ~CPUFP_BIT(28); /* OS support alone is insufficient. */
    assert((cpufp_decode_features(&s) & ~(baseline | amx)) == 0);

    for (unsigned bit = 5; bit <= 7; ++bit) {
        s = full_state();
        s.xcr0 &= ~(UINT64_C(1) << bit);
        f = cpufp_decode_features(&s);
        assert(f & CPUFP_AVX);
        assert(!(f & (CPUFP_AVX512F | CPUFP_AVX512VL | CPUFP_AVX512_VNNI |
                      CPUFP_AVX512_BF16 | CPUFP_AVX512_FP16)));
    }
    s = full_state();
    s.leaf7.ebx &= ~CPUFP_BIT(16);
    assert(!(cpufp_decode_features(&s) & CPUFP_AVX512_VNNI));
    s = full_state();
    s.leaf7.ebx &= ~CPUFP_BIT(31);
    f = cpufp_decode_features(&s);
    assert(cpufp_feature_available(f, "AVX512_VNNI", "512b"));
    assert(!cpufp_feature_available(f, "AVX512_VNNI", "256b"));
    assert(!cpufp_feature_available(f, "AVX512_BF16", "128b"));
    assert(!cpufp_feature_available(f, "AVX512_FP16", "256b"));

    s = full_state();
    s.amx_permission = 0;
    f = cpufp_decode_features(&s);
    assert(!(f & amx));
    assert(f & CPUFP_AVX512F);
    for (unsigned bit = 17; bit <= 18; ++bit) {
        s = full_state();
        s.xcr0 &= ~(UINT64_C(1) << bit);
        assert(!(cpufp_decode_features(&s) & amx));
    }
    s = full_state();
    s.leaf7.edx &= ~CPUFP_BIT(24);
    assert(!(cpufp_decode_features(&s) & amx));
    s = full_state();
    s.leaf7.ebx &= ~CPUFP_BIT(5); /* AVX2 initialization instructions. */
    f = cpufp_decode_features(&s);
    assert(!(f & (CPUFP_AVX_VNNI | CPUFP_AVX_VNNI_INT8 | CPUFP_AVX_VNNI_INT16)));
    assert(!cpufp_feature_available(f, "AVX512_VNNI", "256b"));
    assert(cpufp_feature_available(f, "AVX512_VNNI", "128b"));

    s = full_state();
    s.leaf7_1 = (struct cpufp_cpuid_regs){0}; /* Unsupported subleaf. */
    f = cpufp_decode_features(&s);
    assert(!(f & (CPUFP_AVX_VNNI | CPUFP_AVX_VNNI_INT8 | CPUFP_AVX_VNNI_INT16 |
                  CPUFP_AVX512_BF16 | CPUFP_AMX_FP16)));
    assert(!cpufp_feature_available(f, "unknown", "512b"));
    puts("x64 feature regression tests passed");
    return 0;
}
