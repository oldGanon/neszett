#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uint32_t b32;

#include <stddef.h>

typedef uintptr_t mi;

// typedef uintptr_t uptr;
// typedef size_t size;

#define global static

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define RGB2U32(R,G,B) RGBA2U32(R,G,B,1.0f)
#define RGBA2U32(R,G,B,A) ((u32)(Saturate(R) * 255.0f) <<  0 | \
                           (u32)(Saturate(G) * 255.0f) <<  8 | \
                           (u32)(Saturate(B) * 255.0f) << 16 | \
                           (u32)(Saturate(A) * 255.0f) << 24)

#define MACRO_STRING(S) STRINGIFY(S)
#define STRINGIFY(S) #S

struct string
{
    const char *Data;
    mi Length;
};

/* API */
inline void* Api_Malloc(mi Size);
inline void* Api_Talloc(mi Size);
inline void* Api_Palloc(mi Size);
inline void* Api_Realloc(void *Ptr, mi Size);
inline void Api_Free(void *Memory);
inline void Api_PrintString(u32 Target, string String);
#define Api_Print(S)   Api_PrintString(0, S)
#define Api_Error(S)   Api_PrintString(1, S)
#define Api_Warning(S) Api_PrintString(2, S)
inline u8 Api_GetGamepad();

/* ATOMICS */
struct atomic;
inline u32 Atomic_Inc(atomic *Val);
inline u32 Atomic_Dec(atomic *Val);
inline u32 Atomic_Get(atomic *Val);
inline u32 Atomic_Add(atomic *Val, u32 Add);
inline u32 Atomic_Sub(atomic *Val, u32 Sub);
inline u32 Atomic_Set(atomic *Val, u32 New);
inline u32 Atomic_Cas(atomic *Val, u32 New, u32 Old);

/* THREAD */
struct mutex { volatile mi Handle; };
struct cond { volatile mi Handle; };
struct sem { volatile mi Handle; };

inline mutex Mutex_Create();
inline cond Cond_Create();
inline sem Sem_Create(u32 Value);
inline void Mutex_Destroy(mutex Mutex);
inline void Cond_Destroy(cond Cond);
inline void Sem_Destroy(sem Sem);

inline void Mutex_Lock(mutex Mutex);
inline void Mutex_Unlock(mutex Mutex);
inline void Cond_Signal(cond Cond);
inline void Cond_Wait(cond Cond, mutex Mutex);
inline void Sem_Post(sem Sem);
inline void Sem_Wait(sem Sem);
inline b32 Sem_TryWait(sem Sem);
inline u32 Sem_Value(sem Sem);

struct thread { mi Handle; };
typedef i32 thread_func(void *Data);
static thread Thread_Create(thread_func ThreadFunc, void *Data);
static void Thread_Wait(thread Thread);

/* FILE */
struct file { mi Handle; };
static file File_Open(string File);
static file File_OpenEmpty(string Filename);
static void File_Close(file File);
static b32 File_Valid(file File);
static mi File_Size(file File);
static u8 File_ReadByte(file File, mi Offset);
static void File_WriteByte(file File, mi Offset, u8 Byte);
static b32 File_ReadData(file File, mi Offset, void *Data, mi Bytes);
static b32 File_WriteData(file File, mi Offset, const void *Data, mi Bytes);
inline void* File_ReadEntireFile(string Filename, mi *Bytes = 0);
inline void File_WriteEntireFile(string Filename, const void *Data, mi Bytes);
