inline i64
i32_Compare(const i32 A, const i32 B)
{
    return ((i64)A - (i64)B);
}

template <typename t> inline void
Swap(t *A, t *B)
{
    t T = *A;
    *A = *B;
    *B = T;
}

template <typename t> static void
Sort_Quick(t *Array, mi Count, i64(*Compare)(const t, const t))
{
    if (Count <= 1) return;
    t Pivot = Array[0];

#if 1
    mi i = 0;
    mi j = Count;
    for (;;)
    {
        while (++i < Count && Compare(Array[i], Pivot) <= 0);
        while (Compare(Array[--j], Pivot) > 0);
        if (i >= j) break;
        Swap(Array + i, Array + j);
    }
#else
    mi j = 0;
    for (mi i = 1; i < Count; ++i)   
    if (Compare(Array[i], Pivot) < 0)
        Swap(Array + i, Array + (++j));
#endif

    Swap(Array, Array + j);
    Sort_Quick(Array,         j,             Compare);
    Sort_Quick(Array + j + 1, Count - j - 1, Compare);
}

template <typename t> static void
Sort_Bubble(t *Array, mi Count, i64(*Compare)(const t, const t))
{
    if (Count <= 1) return;
    for (mi i = 0; i < Count - 1; ++i)
    for (mi j = i + 1; j < Count; ++j)
        if (Compare(Array[i], Array[j]) > 0)
            Swap(Array + i, Array + j);
}
