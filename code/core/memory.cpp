#include <string.h>

//
// MEMORY FUNCTIONS
//

static void
Memory_Copy(void *Dst, const void *Src, mi Size)
{
    memcpy(Dst, Src, Size);
}

static void
Memory_Set(void *Ptr, u8 Value, mi Size)
{
    memset(Ptr, Value, Size);
}

static int
Memory_Compare(const void *S1, const void *S2, mi Size)
{
    return memcmp(S1, S2, Size);
}

static void *
Memory_Move(void *Dst, const void *Src, mi Size)
{
    if (Size == 0 || Dst == Src)
        return Dst;
    
    char *Dst8 = (char *)Dst;
    const char *Src8 = (const char *)Src;
    if ((u8 *)Dst8 < (u8 *)Src8)
        while (Size--) { *Dst8++ = *Src8++; }
    else
        while (Size--) { Dst8[Size] = Src8[Size]; }

    return Dst;
}

//
// MEMORY ZONE
//

#define MINFRAGMENT 64
struct memory_block
{
    memory_block *Next, *Prev;
    mi Size;
    u64 Tag;
};

struct memory_zone
{
    memory_block Blocklist;
    memory_block *Sentinel;
    mi Size;
    mi SizeLeft;
    mi Padding;
};

inline memory_zone* 
Zone_Clear(memory_zone *Zone, mi Size)
{
    memory_block *Block;

    Block = Zone->Blocklist.Next = Zone->Blocklist.Prev =
        (memory_block *)((u8 *)Zone + sizeof(memory_zone));
    Zone->Size = Size - sizeof(memory_zone);
    Zone->SizeLeft = Zone->Size;
    Zone->Blocklist.Tag = 1;
    Zone->Blocklist.Size = 0;
    Zone->Sentinel = Block;

    Block->Prev = Block->Next = &Zone->Blocklist;
    Block->Tag = 0;
    Block->Size = Size - sizeof(memory_zone);

    return Zone;
}

inline void 
Zone_Free(memory_zone *Zone, const void *Ptr)
{
    if (!Ptr) return;
    if (Ptr < Zone || Ptr > Zone + Zone->Size) return;

    memory_block *Block = (memory_block *)((u8 *)Ptr - sizeof(memory_block));

    if (Block->Tag == 0) return;

    Zone->SizeLeft += Block->Size;

    Block->Tag = 0;

    memory_block *Other = Block->Prev;
    if (!Other->Tag)
    {
        Other->Size += Block->Size;
        Other->Next = Block->Next;
        Other->Next->Prev = Other;
        if (Block == Zone->Sentinel)
            Zone->Sentinel = Other;
        Block = Other;
    }

    Other = Block->Next;
    if (!Other->Tag)
    {
        Block->Size += Other->Size;
        Block->Next = Other->Next;
        Block->Next->Prev = Block;
        if (Other == Zone->Sentinel)
            Zone->Sentinel = Block;
    }
}

inline void
Zone_SplitBlock(memory_block *Base, mi NewSize)
{
    mi Extra = Base->Size - NewSize;
    if (Extra > MINFRAGMENT)
    {
        memory_block *New;
        New = (memory_block *)((u8 *)Base + NewSize);
        New->Size = Extra;
        New->Tag = 0;
        New->Prev = Base;
        New->Next = Base->Next;
        New->Next->Prev = New;
        Base->Next = New;
        Base->Size = NewSize;
    }
}

inline void*
Zone_TagMalloc(memory_zone *Zone, mi Size, u64 Tag = 1)
{
    // add header size and align to 16-byte
    Size += sizeof(memory_block);
    Size = (Size + 15) & ~15;

    memory_block *Base = Zone->Sentinel;
    memory_block *Scanner = Zone->Sentinel;
    memory_block *Start = Base->Prev;

    // scan through the block list 
    // looking for the first free block of sufficient size
    do
    {
        if (Scanner == Start) return 0;
        if (Scanner->Tag) Base = Scanner = Scanner->Next;
        else Scanner = Scanner->Next;

    } while (Base->Tag || Base->Size < Size);

    // found a big enough block
    // only split block if large enough
    Zone_SplitBlock(Base, Size);

    Zone->SizeLeft -= Base->Size;

    Base->Tag = Tag;

    Zone->Sentinel = Base->Next;

    void *Buffer = (void *)((u8 *)Base + sizeof(memory_block));
    Memory_Set(Buffer, 0, Base->Size - sizeof(memory_block));
    return Buffer;
}

inline void*
Zone_Malloc(memory_zone *Zone, mi Size)
{
    return Zone_TagMalloc(Zone, Size, 1);
}

inline void*
Zone_Realloc(memory_zone *Zone, void *Ptr, mi Size)
{
    if (!Ptr)
        return Zone_Malloc(Zone, Size);

    memory_block *Block = (memory_block *)((u8 *)Ptr - sizeof(memory_block));
    memory_block *Next = Block->Next;
    b32 Sentinel = (Next == Zone->Sentinel);

    mi NewSize = Size;
    mi OldSize = Block->Size - sizeof(memory_block);

    Size += sizeof(memory_block);
    Size = (Size + 15) & ~15;

    if (Block->Size == Size)
        return Ptr;

    if ((Size < Block->Size) || 
        ((Next->Tag == 0) && (Block->Size + Next->Size >= Size)))
    {
        Zone->SizeLeft += Block->Size;

        if (Next->Tag == 0)
        {
            Block->Size += Next->Size;
            Block->Next = Next->Next;
            Block->Next->Prev = Block;
        }

        Zone_SplitBlock(Block, Size);
        
        if (Sentinel)
            Zone->Sentinel = Block->Next;

        Zone->SizeLeft -= Block->Size;

        return Ptr;
    }

    void *New = Zone_Malloc(Zone, NewSize);
    Memory_Copy(New, Ptr, OldSize);
    Zone_Free(Zone, Ptr);
    return New;
}

//
// MEMORY ARENA
//
struct memory_arena
{
    mi Size;
    mi Used;
    i32 MarkerCount;
};

struct memory_arena_marker
{
    memory_arena *Arena;
    mi Used;
};

inline memory_arena*
Arena_Clear(memory_arena *Arena, mi Size)
{
    Arena->Size = Size;
    Arena->Used = sizeof(memory_arena);
    Arena->MarkerCount = 0;

    return Arena;
}

inline mi
Arena_GetAlignmentOffset(memory_arena *Arena, mi Alignment)
{
    mi AlignmentOffset = 0;
    
    mi ResultPointer = (mi)Arena + Arena->Used;
    mi AlignmentMask = Alignment - 1;
    if (ResultPointer & AlignmentMask)
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);

    return AlignmentOffset;
}

inline b32
Arena_HasSpace(memory_arena *Arena, mi Size, mi Alignment = 16)
{
    Size += Arena_GetAlignmentOffset(Arena, Alignment);
    return ((Arena->Used + Size) <= Arena->Size);
}

#define Arena_PushStruct(Arena, type, ...) (type *)Arena_PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define Arena_PushArray(Arena, Count, type, ...) (type *)Arena_PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define Arena_PushSize(Arena, Size, ...) Arena_PushSize_(Arena, Size, ## __VA_ARGS__)
inline void*
Arena_PushSize_(memory_arena *Arena, mi SizeInit, mi Alignment = 16)
{
    void *Result = 0;
    mi Size = SizeInit;
    
    mi AlignmentOffset = Arena_GetAlignmentOffset(Arena, Alignment);
    Size += AlignmentOffset;
    
    Assert((Arena->Used + Size) <= Arena->Size);

    Result = ((u8 *)Arena) + Arena->Used + AlignmentOffset;
    Arena->Used += Size;

    return Result;
}

inline memory_arena_marker
Arena_PlaceArenaMarker(memory_arena *Arena)
{
    memory_arena_marker Marker;

    Marker.Arena = Arena;
    Marker.Used = Arena->Used;

    ++Arena->MarkerCount;

    return Marker;
}

inline void
Arena_RevertToArenaMarker(memory_arena_marker Marker)
{
    memory_arena *Arena = Marker.Arena;
    Assert(Arena->Used >= Marker.Used);
    Arena->Used = Marker.Used;
    Assert(Arena->MarkerCount > 0);
    --Arena->MarkerCount;
}

#define ARENA_STACK_MARKER(Arena) memory_arena_stack_marker StackMarker(Arena);
struct memory_arena_stack_marker
{
    memory_arena_marker Marker;
    
    memory_arena_stack_marker(memory_arena *Arena)
    {
        Marker = Arena_PlaceArenaMarker(Arena);
    }
    
    ~memory_arena_stack_marker()
    {
        Arena_RevertToArenaMarker(Marker);
    }
};
