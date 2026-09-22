// Exercise real registration and table counts on any host, without SIMD execution.
#define _SSE_
#define _SSE2_
#define _FMA_
#define _AVX512_VNNI_
#define _AMX_FP16_
#define main cpufp_program_main
#include "../x64/cpufp.cpp"
#undef main
#include <cassert>

static uint32_t detected_features;
extern "C" uint32_t cpufp_detect_features(int) { return detected_features; }
#define STUB(name) extern "C" void name(int64_t, void *) { std::abort(); }
STUB(sse_128b_add_mul_f32f32_f32)
STUB(sse2_128b_add_mul_f64f64_f64)
STUB(fma_256b_fma_f32f32f32)
STUB(fma_256b_fma_f64f64f64)
STUB(fma_128b_fma_f32f32f32)
STUB(fma_128b_fma_f64f64f64)
STUB(amx_fp16_mm_f32f16f16)
STUB(avx512_vnni_512b_dp4a_s32u8s8)
STUB(avx512_vnni_256b_dp4a_s32u8s8)
STUB(avx512_vnni_128b_dp4a_s32u8s8)
STUB(avx512_vnni_512b_dp2a_s32s16s16)
STUB(avx512_vnni_256b_dp2a_s32s16s16)
STUB(avx512_vnni_128b_dp2a_s32s16s16)

static void check(uint32_t features, int dsa, int wide, int medium, int narrow)
{
    detected_features = features;
    bm_list.clear();
    num_dsa = num_simd_512b = num_simd_256b = num_simd_128b = 0;
    cpufp_register_isa();
    assert(num_dsa == dsa && num_simd_512b == wide);
    assert(num_simd_256b == medium && num_simd_128b == narrow);
    assert(bm_list.size() == static_cast<size_t>(dsa + wide + medium + narrow));
    size_t i = 0;
    for (int n = 0; n < dsa; ++n) assert(bm_list[i++].vlen == "DSA");
    for (int n = 0; n < wide; ++n) assert(bm_list[i++].vlen == "512b");
    for (int n = 0; n < medium; ++n) assert(bm_list[i++].vlen == "256b");
    for (int n = 0; n < narrow; ++n) assert(bm_list[i++].vlen == "128b");
}

int main()
{
    check(0, 0, 0, 0, 0);
    check(CPUFP_SSE | CPUFP_SSE2, 0, 0, 0, 2);
    check(CPUFP_FMA, 0, 0, 2, 2);
    check(CPUFP_AMX_FP16, 1, 0, 0, 0);
    check(CPUFP_AVX512_VNNI, 0, 2, 0, 0);
    check(0xffffU, 1, 2, 4, 6);
    puts("x64 runtime registration tests passed");
}
