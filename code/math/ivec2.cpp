//
// 2 Element Integer Vector
//

union ivec2
{
    struct
    {
        i32 x, y;
    };
    struct
    {
        i32 u, v;
    };
    i32 E[2];
};

// iv2 constructors
inline ivec2 iVec2() { return { }; }
inline ivec2 iVec2(i32 r) { return {r, r}; }
inline ivec2 iVec2(i32 x, i32 y) { return {x, y}; }
inline ivec2 iVec2(f32 x, f32 y) { return {(i32)x, (i32)y}; }

// iv2 arithmetic
inline ivec2 operator+ (const ivec2 A, const ivec2 B) { return { A.x + B.x, A.y + B.y }; }
inline ivec2 operator- (const ivec2 A, const ivec2 B) { return { A.x - B.x, A.y - B.y }; }
inline ivec2 operator* (const ivec2 A, const ivec2 B) { return { A.x * B.x, A.y * B.y }; }
inline ivec2 operator/ (const ivec2 A, const ivec2 B) { return { A.x / B.x, A.y / B.y }; }

inline ivec2 operator+ (const ivec2 A, const i32 B) { return { A.x + B, A.y + B }; }
inline ivec2 operator- (const ivec2 A, const i32 B) { return { A.x - B, A.y - B }; }
inline ivec2 operator* (const ivec2 A, const i32 B) { return { A.x * B, A.y * B }; }
inline ivec2 operator/ (const ivec2 A, const i32 B) { return { A.x / B, A.y / B }; }

inline void operator+= (ivec2 &A, const ivec2 B) { A.x += B.x, A.y += B.y; }
inline void operator-= (ivec2 &A, const ivec2 B) { A.x -= B.x, A.y -= B.y; }
inline void operator*= (ivec2 &A, const ivec2 B) { A.x *= B.x, A.y *= B.y; }
inline void operator/= (ivec2 &A, const ivec2 B) { A.x /= B.x, A.y /= B.y; }

// iv2 unary
inline ivec2 operator- (const ivec2 A) { return { -A.x, -A.y }; }

inline ivec2
Clamp(ivec2 V, ivec2 Min, ivec2 Max)
{
    return iVec2(CLAMP(V.x, Min.x, Max.x), CLAMP(V.y, Min.y, Max.y));
}

inline ivec2
Abs(ivec2 A)
{
    return iVec2(ABS(A.x), ABS(A.y));
}

inline ivec2
Min(ivec2 A, ivec2 B)
{
    return iVec2(MIN(A.x, B.x), MIN(A.y, B.y));
}

inline ivec2
Max(ivec2 A, ivec2 B)
{
    return iVec2(MAX(A.x, B.x), MAX(A.y, B.y));
}

inline ivec2
Sign(ivec2 A)
{
    return { (A.x<0)?-1:1,(A.y<0)?-1:1 };
}
