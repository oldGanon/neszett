#include "config.h"
#include "platform.h"

#define ORG_NAME "oldGanon"
#define GAME_NAME "neszett"
#define VERSION 0

struct cpuid
{
    b32 HasRDTSC;
    b32 HasSSE;
    b32 HasSSE2;
    b32 HasSSE3;
    b32 HasSSSE3;
    b32 HasSSE41;
    b32 HasSSE42;
    b32 HasFMA;
    b32 HasAVX;
};

global cpuid CPUID;

/* INTRINSICS */
#if COMPILE_INTRINSICS
    #if COMPILE_X64 || COMPILE_X86
        #if COMPILE_LLVM
            #include <x86intrin.h>
        #elif COMPILE_MSVC
            #include <intrin.h>
        #else
            #error unsupported compiler!
        #endif
    #elif COMPILE_NEON
        #include <arm_neon.h>
    #endif
#endif

/* OS */
#if COMPILE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#endif

/* OS LAYER */
#if COMPILE_SDL
    #include "sdl.cpp"
#elif COMPILE_WIN32
    #include "win32.cpp"
#else
    #error unsupported platform!
#endif

#if COMPILE_WINDOWS
void WinMainCRTStartup()
{

#if COMPILE_X86
    #if COMPILE_MSVC
        i32 CPUID0[4] = { -1 };
        __cpuid(CPUID0, 0);
        Swap(CPUID0 + 2, CPUID0 + 3);
        if (CPUID0[0] > 0)
        {
            i32 CPUID1[4] = { -1 };
            __cpuid(CPUID1, 1);
            CPUID.HasRDTSC = (CPUID1[3] & (1 <<  4)) != 0;
            CPUID.HasSSE   = (CPUID1[3] & (1 << 25)) != 0;
            CPUID.HasSSE2  = (CPUID1[3] & (1 << 26)) != 0;
            CPUID.HasSSE3  = (CPUID1[2] & (1 <<  0)) != 0;
            CPUID.HasSSSE3 = (CPUID1[2] & (1 <<  9)) != 0;
            CPUID.HasSSE41 = (CPUID1[2] & (1 << 19)) != 0;
            CPUID.HasSSE42 = (CPUID1[2] & (1 << 20)) != 0;
            CPUID.HasFMA   = (CPUID1[2] & (1 << 12)) != 0;
            CPUID.HasAVX   = (CPUID1[2] & (1 << 28)) != 0;
        }
    #endif

    #if COMPILE_X64
        u32 MXCSR = _mm_getcsr();
        MXCSR = _MM_ROUND_NEAREST | _MM_FLUSH_ZERO_ON | _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | 
                _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT;
        _mm_setcsr(MXCSR);
    #endif
#endif

    int Result = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(Result);
}
#endif /* COMPILE_WINDOWS */

extern "C" int _fltused = 0;
extern "C"
{
    #pragma function(memset)
    void *memset(void *dst, int c, size_t count)
    {
        char *bytes = (char *)dst;
        while (count--) { *bytes++ = (char)c; }
        return dst;
    }

    #pragma function(memcpy)
    void *memcpy(void *dst, const void *src, size_t count)
    {
        char *dst8 = (char *)dst;
        const char *src8 = (const char *)src;
        while (count--) { *dst8++ = *src8++; }
        return dst;
    }

    #pragma function(memcmp)
    int memcmp(const void *s1, const void *s2, size_t count)
    {
        const unsigned char *p1 = (const unsigned char *)s1;
        const unsigned char *p2 = (const unsigned char *)s2;
        for (; count--; p1++, p2++){ if (*p1 != *p2) return *p1 - *p2; }
        return 0;
    }

    #pragma function(strcmp)
    int strcmp(const char* s1, const char* s2)
    {
        while(*s1 && (*s1 == *s2)) { s1++; s2++; }
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
    }
}
