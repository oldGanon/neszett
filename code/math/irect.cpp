struct irect
{
    ivec2 Min, Max;
};

inline irect
iRect()
{
    return { };
}

inline irect
iRect(i32 X0, i32 Y0, i32 X1, i32 Y1)
{
    return { { X0, Y0 }, { X1, Y1 } };
}

inline irect
iRect(ivec2 Min, ivec2 Max)
{
    return { Min, Max };
}

inline irect operator+ (const irect A, const ivec2 B) { return { A.Min + B, A.Max + B }; }

inline ivec2 Dim(irect R){ return R.Max - R.Min; }
inline ivec2 Min(irect R){ return R.Min; }
inline ivec2 Max(irect R){ return R.Max; }

static b32
Inside(irect R, ivec2 P)
{
    if (P.x < R.Min.x || P.y < R.Min.y ||
        P.x >= R.Max.x || P.y >= R.Max.y)
        return false;
    return true;
}

static irect
Intersect(irect A, irect B)
{
    return { MAX(A.Min.x, B.Min.x),
             MAX(A.Min.y, B.Min.y),
             MIN(A.Max.x, B.Max.x),
             MIN(A.Max.y, B.Max.y) };
}
