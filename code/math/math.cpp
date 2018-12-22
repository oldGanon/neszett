// #include <math.h>

#define PI 3.1415926535897932384626433832795f

inline u32
Cast_F32toU32(f32 f)
{
    // return (*(u32*)&f);
    union { f32 f; u32 u; } fu = { f };
    return fu.u;
}

inline f32
Cast_U32toF32(u32 u)
{
    // return (*(f32*)&u);
    union { u32 u; f32 f; } fu = { u };
    return fu.f;
}

inline f32
Round(f32 n)
{
#if COMPILE_X64
    return _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_set1_ps(n))));
#else
    return roundf(n);
#endif
}

inline f32
Truncate(f32 n)
{
#if COMPILE_X64
    return _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_set1_ps(n))));
#else
    return truncf(n);
#endif
}

inline f32
Ceil(f32 n)
{
#if COMPILE_SSE
    return _mm_cvtss_f32(_mm_ceil_ps(_mm_set1_ps(n)));
#elif COMPILE_X64
    __m128 F = _mm_set1_ps(n);;
    __m128 T = _mm_cvtepi32_ps(_mm_cvttps_epi32(F));
    return _mm_cvtss_f32(_mm_add_ps(T, _mm_and_ps(_mm_cmpgt_ps(F, T), _mm_set_ss(1.0f))));
#else
    return ceilf(n);
#endif
}

inline f32
Floor(f32 n)
{
#if COMPILE_SSE
    return _mm_cvtss_f32(_mm_floor_ps(_mm_set1_ps(n)));
#elif COMPILE_X64
    __m128 F = _mm_set1_ps(n);
    __m128 T = _mm_cvtepi32_ps(_mm_cvttps_epi32(F));
    return _mm_cvtss_f32(_mm_sub_ps(T, _mm_and_ps(_mm_cmplt_ps(F, T), _mm_set_ss(1.0f))));
#else
    return floorf(n);
#endif
}

inline f32
Fract(f32 n)
{
    return n - Floor(n);
}

inline f32
Modulo(f32 n, f32 d)
{
    return n - d * Floor(n / d);
}

inline f32
SquareRoot(f32 n)
{
#if COMPILE_X64
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set1_ps(n)));
#else
    return sqrtf(n);
#endif
}

inline f32 
DegToRad(f32 AngDeg)
{
    const f32 DegToRad = 3.14159f * 2.0f / 360.0f;
    return AngDeg * DegToRad;
}

inline f32 
RadToDeg(f32 AngRad)
{
    const f32 RadToDeg = 360.0f / (3.14159f * 2.0f);
    return  AngRad * RadToDeg;
}

inline f32
Lerp(f32 r0, f32 r1, f32 t) {
    return (1.0f-t)*r0 + t*r1;
}

inline f32
Clamp(f32 n, f32 t0, f32 t1)
{
    if(n < t0) return t0;
    if(n > t1) return t1;
    return n;
}

inline f32
Saturate(f32 n)
{
    if(n < 0.0f) return 0.0f;
    if(n > 1.0f) return 1.0f;
    return n;
}

inline f32 Sign(f32 r) { return (r<0)?-1.0f:1.0f; }
inline f32 Positive(f32 r) { return (r<0)?-r:r; }
inline f32 Negative(f32 r) { return (r>0)?-r:r; }
inline f32 Absolute(f32 n) { return (n<0)?-n:n; }
inline f32 Norm(f32 n) { return Absolute(n); }
inline f32 Max(f32 a, f32 b) { return (a>b)?a:b; }
inline f32 Min(f32 a, f32 b) { return (a<b)?a:b; }

#define ABS(a) ((a<0)?-a:a)
#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)
#define CLAMP(r,a,b) ((r<a)?a:(r>b)?b:r)
#define SATURATE(r)  CLAMP(r,0,1)

inline u32
FirstSetMSB(u64 Mask)
{
#if COMPILE_MSVC && COMPILE_INTRINSICS
    unsigned long i;
    if (_BitScanReverse64(&i, Mask)) return i;
    else return 0xFFFFFFFF;
#else
    i32 b = 0;
    if (Mask>((u64)1<<31)){Mask>>=32;b =32;}
    if (Mask>((u64)1<<15)){Mask>>=16;b|=16;}
    if (Mask>((u64)1<< 7)){Mask>>= 8;b|= 8;}
    if (Mask>((u64)1<< 3)){Mask>>= 4;b|= 4;}
    if (Mask>((u64)1<< 1)){Mask>>= 2;b|= 2;}
    if (Mask>((u64)1<< 0)){          b|= 1;}
    return b;
#endif
}

// inline u32
// FirstSetLSB(u64 Mask)
// {
// #if COMPILE_MSVC
//     unsigned long i;
//     if (_BitScanForward64(&i, Mask)) return i;
//     else return 0xFFFFFFFF;
// #else
//     #error
// #endif
// }

inline i32
Log2(u64 n)
{
    return FirstSetMSB(n);
}

inline u64
CeilToPowerOf2(u64 n)
{
    i32 i = Log2(n - 1) + 1;
    return ((u64)1 << i);
}

//
// TRIGONOMETRY
//

inline f32
Sin(f32 x)
{
    x = x * (1.0f/PI) + 1.0f;
    x = Modulo(x, 2.0f) - 1.0f;
    f32 x2 = x * x;
    f32 x3 =       0.000385937753182769f;
    x3 = x3 * x2 - 0.006860187425683514f;
    x3 = x3 * x2 + 0.0751872634325299f;
    x3 = x3 * x2 - 0.5240361513980939f;
    x3 = x3 * x2 + 2.0261194642649887f;
    x3 = x3 * x2 - 3.1415926444234477f;
    return (x-1.0f) * (x+1.0f) * x3 * x;
}

inline f32
Cos(f32 x)
{
    x = x * (1.0f/PI) + 1.5f;
    x = Modulo(x, 2.0f) - 1.0f;
    f32 x2 = x * x;
    f32 x3 =       0.000385937753182769f;
    x3 = x3 * x2 - 0.006860187425683514f;
    x3 = x3 * x2 + 0.0751872634325299f;
    x3 = x3 * x2 - 0.5240361513980939f;
    x3 = x3 * x2 + 2.0261194642649887f;
    x3 = x3 * x2 - 3.1415926444234477f;
    return (x-1.0f) * (x+1.0f) * x3 * x;
}

inline f32
Tan(f32 x)
{
    return Sin(x) / Cos(x);
}

static f32
Sinc(f32 X)
{
    return (X == 0.0f) ? 1.0f : Sin(X*PI)/(X*PI);
}

//
//
//

#include "ivec2.cpp"
#include "irect.cpp"
