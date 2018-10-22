//
// STRING
//

b32 operator==(string A, string B)
{
    if (A.Length != B.Length) return false;
    for (mi i = 0; i < A.Length; ++i)
        if (A.Data[i] != B.Data[i])
            return false;
    return true;
}

string operator ""_string(const char *Data, size_t Length) { return { Data, Length }; };
#define S(S) "" S ""_string

static mi
String_Length(const char *Str)
{
    if (!Str) return 0;    
    mi Result = 0;
    while (*Str++) ++Result;
    return Result;
}

inline string
String()
{
    return { };
}

inline string
String(const char *Str)
{
    return { Str, String_Length(Str) };
}

inline string
String(const char *Str, mi Length)
{
    return { Str, Length };
}

inline string
String_Alloc(string Str)
{
    char *Data = (char *)Api_Malloc(Str.Length);
    Memory_Copy(Data, Str.Data, Str.Length);
    return String(Data, Str.Length);
}

static i32
String_Compare(string A, string B)
{
    if (A.Length != B.Length)
        return (A.Length > B.Length) ? A.Data[B.Length] : -B.Data[A.Length];

    for (mi i = 0; i < A.Length; ++i)
        if (A.Data[i] != B.Data[i])
            return A.Data[i] - B.Data[i];

    return 0;
}

static string
String_SubString(string Str, mi Start, mi End)
{
    if (Start > Str.Length) Start = Str.Length;
    if (End > Str.Length) End = Str.Length;
    if (End < Start) End = Start;
    Str.Data += Start;
    Str.Length = End - Start;
    return Str;
}

static b32
String_StartsWith(string Str, string Match)
{
    if (Str.Length < Match.Length)
        return false;

    Str = String_SubString(Str, 0, Match.Length);
    return !String_Compare(Str, Match);
}

static b32
String_EndsWith(string Str, string Match)
{
    if (Str.Length < Match.Length)
        return false;

    Str = String_SubString(Str, Str.Length - Match.Length, Str.Length);
    return !String_Compare(Str, Match);
}

static mi
String_Count(string Str, const char Match)
{
    mi Result = 0;
    for (mi i = 0; i < Str.Length; ++i)
        if (Str.Data[i] == Match)
            ++Result;
    return Result;
}

static mi
String_Count(string Str, string Match)
{
    if (Str.Length < Match.Length)
        return 0;

    mi Result = 0;
    for (mi i = 0; i < Str.Length; ++i)
    {
        for (mi j = 0; j < Match.Length; ++j)
            if (Str.Data[i+j] != Match.Data[j])
                break;
        ++Result;
    }
    return Result;
}

static b32
String_Contains(string Str, const char Match)
{
    for (mi i = 0; i < Str.Length; ++i)
        if (Str.Data[i] == Match)
            return true;
    return false;
}

static b32
String_Contains(string Str, string Match)
{
    if (Str.Length < Match.Length)
        return false;

    for (mi i = 0; i < Str.Length; ++i)
    {
        for (mi j = 0; j < Match.Length; ++j)
            if (Str.Data[i+j] != Match.Data[j])
                break;
        return true;
    }
    return false;
}

static string
String_SplitLeft(string *Str, const char Match)
{
    string Left = *Str;
    for (mi i = 0; i < Str->Length; ++i)
    {
        if (Str->Data[i] == Match)
        {
            Left.Length = i++;
            Str->Data += i;
            Str->Length -= i;
            return Left;
        }
    }
    Str->Data += Str->Length;
    Str->Length = 0;
    return Left;
}

static string
String_SplitLeft(string *Str, string Match)
{
    string Left = *Str;
    Left.Length = 0;
    while (Str->Length > Match.Length)
    {
        if (String_StartsWith(*Str, Match))
        {
            Str->Data += Match.Length;
            Str->Length -= Match.Length;
            return Left;
        }

        Left.Length++;
        Str->Data++;
        Str->Length--;
    }
    Left.Length += Str->Length;
    Str->Data += Str->Length;
    Str->Length = 0;
    return Left;
}

static string
String_SplitRight(string *Str, const char Match)
{
    string Right = *Str;
    for (mi i = 0; i < Str->Length; ++i)
    {
        mi j = Str->Length - i;
        if (Str->Data[j] == Match)
        {
            Str->Length = j++;
            Right.Data += j;
            Right.Length -= j;
            return Right;
        }
    }
    Str->Length = 0;
    return Right;
}

//
// UTF-8
//

global const u8 UTF8LUP[256] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,
};

inline string
UTF8_GetCharacter(string Str)
{
    u8 ByteCount = UTF8LUP[(u8)Str.Data[0]];
    return String(Str.Data, ByteCount);
}

inline string
UTF8_GetLastCharacter(string Str)
{
    mi n = Str.Length;
    while ((Str.Data[--n] & 0xC) == 0x80);
    u8 ByteCount = UTF8LUP[(u8)Str.Data[n]];
    return String(Str.Data + n, ByteCount);
}

inline string
UTF8_ExtractCharacter(string *Str)
{
    string Result = UTF8_GetCharacter(*Str);
    Str->Data += Result.Length;
    Str->Length -= Result.Length;
    return Result;
}

inline u32
UTF8_GetCodepoint(string Str)
{
    u8 Byte = Str.Data[0];
    if (Byte < 128) return Byte;
    u32 K = 63 - FirstSetMSB(~((u64)Byte << 56));
    u32 Value = ((u32)Byte & (0xFF >> K));
    switch (K)
    {
        case 4:  Value <<= 6; Value |= Str.Data[1] & 0x3F;
        case 3:  Value <<= 6; Value |= Str.Data[2] & 0x3F;
        default: Value <<= 6; Value |= Str.Data[3] & 0x3F;
    }
    return Value;
}

inline u32
UTF8_ExtractCodepoint(string *Str)
{
    string Code = UTF8_ExtractCharacter(Str);
    return UTF8_GetCodepoint(Code);
}

inline b32
UTF8_IsWhiteSpace(u32 Codepoint)
{
    if (Codepoint == 9 ||  Codepoint == 10 ||
        Codepoint == 11 || Codepoint == 13 ||
        Codepoint == 32 || Codepoint == 133 ||
        Codepoint == 160)
        return true;
#if 0
    if (Codepoint == 5760 || Codepoint == 8192 ||
        Codepoint == 8193 || Codepoint == 8194 ||
        Codepoint == 8195 || Codepoint == 8196 ||
        Codepoint == 8197 || Codepoint == 8198 ||
        Codepoint == 8199 || Codepoint == 8200 ||
        Codepoint == 8201 || Codepoint == 8202 ||
        Codepoint == 8232 || Codepoint == 8233 ||
        Codepoint == 8239 || Codepoint == 8287 ||
        Codepoint == 12288 )
        return true;
#endif
    return false;
}

static string
UTF8_TrimLeft(string Str)
{
    while (Str.Length)
    {
        string Character = UTF8_GetCharacter(Str);
        if (!UTF8_IsWhiteSpace(UTF8_GetCodepoint(Character))) break;
        Str.Data += Character.Length;
        Str.Length -= Character.Length;
    }
    return Str;
}

static string
UTF8_TrimRight(string Str)
{
    while (Str.Length)
    {
        string Character = UTF8_GetLastCharacter(Str);
        if (!UTF8_IsWhiteSpace(UTF8_GetCodepoint(Character))) break;
        Str.Length -= Character.Length;
    }
    return Str;
}

static string
UTF8_Trim(string Str)
{
    return UTF8_TrimRight(UTF8_TrimLeft(Str));
}

static string
UTF8_ExtractToken(string *Str)
{
    *Str = UTF8_TrimLeft(*Str);
    string Token = *Str;
    Token.Length = 0;
    while (Str->Length)
    {
        string Character = UTF8_GetCharacter(*Str);
        if (UTF8_IsWhiteSpace(UTF8_GetCodepoint(Character))) break;
        Str->Data += Character.Length;
        Str->Length -= Character.Length;
        Token.Length += Character.Length;
    }
    *Str = UTF8_TrimLeft(*Str);
    return Token;
}

//
// TEMPORARY STRING OPERATIONS
//

static string
TString(const char *Str)
{
    mi Length = String_Length(Str);
    char *Data = (char *)Api_Talloc(Length);
    Memory_Copy(Data, Str, Length);
    return String(Data, Length);
}

static string
TString_Concat(string A, string B)
{
    string Result;
    Result.Length = A.Length + B.Length;
    char *Data = (char *)Api_Talloc(Result.Length);
    Memory_Copy(Data,            A.Data, A.Length);
    Memory_Copy(Data + A.Length, B.Data, B.Length);
    Result.Data = Data;
    return Result;
}

static string
TString_Concat(string A, string B, string C)
{
    string Result;
    Result.Length = A.Length + B.Length + C.Length;
    char *Data = (char *)Api_Talloc(Result.Length);
    Memory_Copy(Data,                       A.Data, A.Length);
    Memory_Copy(Data + A.Length,            B.Data, B.Length);
    Memory_Copy(Data + A.Length + B.Length, C.Data, C.Length);
    Result.Data = Data;
    return Result;
}

static string
TString_Concat(string A, string B, string C, string D, string E)
{
    string Result;
    Result.Length = A.Length + B.Length + C.Length + D.Length + E.Length;
    char *Data = (char *)Api_Talloc(Result.Length);
    mi Length = 0;
    Memory_Copy(Data + Length, A.Data, A.Length);
    Length += A.Length;
    Memory_Copy(Data + Length, B.Data, B.Length);
    Length += B.Length;    
    Memory_Copy(Data + Length, C.Data, C.Length);
    Length += C.Length;
    Memory_Copy(Data + Length, D.Data, D.Length);
    Length += D.Length;
    Memory_Copy(Data + Length, E.Data, E.Length);
    Result.Data = Data;
    return Result;
}

static string
TString_Null(string Str)
{
    if (Str.Data[Str.Length] == 0) return Str;
    char *Data = (char *)Api_Talloc(Str.Length + 1);
    Memory_Copy(Data, Str.Data, Str.Length);
    Data[Str.Length] = 0;
    return String(Data, Str.Length);
}

#define STB_SPRINTF_MIN 256
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

static char*
TSPrint_Callback(char *String, void *Data, i32 Length)
{
    string *Result = (string *)Data;
    Api_Talloc(Length);
    Memory_Copy((void *)(Result->Data + Result->Length), String, Length);
    Result->Length += Length;
    return String;
}

static string
TSPrintVA(const char *Format, va_list Args)
{
    string Result;
    Result.Data = (const char *)Api_Talloc(0);
    Result.Length = 0;
    char Buffer[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(TSPrint_Callback, (void*)&Result, Buffer, Format, Args);
    return Result;
}

static string
TSPrint(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    string Result = TSPrintVA(Format, Args);
    va_end(Args);
    return Result;
}
