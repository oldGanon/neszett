#ifdef Assert
  #undef Assert
#endif
#define Assert(x) SDL_assert(x);
#if COMPILE_DEV
  #define SDL_ASSERT_LEVEL 3
#else
  #define SDL_ASSERT_LEVEL 0
#endif

#include <SDL/SDL.h>

struct atomic { volatile SDL_atomic_t Atomic; };

#include "core/memory.cpp"
global memory_zone *GlobalZone = 0;
global memory_arena *TempArena = 0;

global string BasePath = { };
global string PrefPath = { };

#include "math/math.cpp"
#include "core/string.cpp"
#include "core/sort.cpp"

#define SYSTEM_HZ 21477272
#define CPU_DIVIDER 12
#define CPU_HZ 1789772.66667 // (SYSTEM_HZ / CPU_DIVIDER)

#if OPENGL_USETEXTUREBUFFER
  global u8 *GlobalScreen;
#else
  global atomic GlobalScreenChanged;
  global u8 GlobalScreen[226][260];
#endif
global SDL_AudioDeviceID GlobalAudioDevice = 0;
global u8 GlobalGamepad;
global atomic GlobalFrame;
global atomic GlobalPhase;

#include "nes/console.cpp"

global b32 GlobalRunning = true;
global u64 GlobalPerfCountFrequency = 0;
global console *GlobalConsole = 0;

#if COMPILE_GFX_OPENGL
  #include <SDL/SDL_opengl.h>
  #define GL_LOAD_FUNC(S) SDL_GL_GetProcAddress(S)
  global SDL_GLContext GlobalGLContext;
#elif COMPILE_GFX_VULKAN
  #include <SDL/SDL_vulkan.h>
  global SDL_vulkanInstance GlobalVKInstance;
#endif

#include "gfx/gfx.cpp"

#include "gamecontrollerdb.cpp"

#include "input.cpp"

struct main_state
{
    thread NESThread;
    
    input Input;

    b32 VSyncState;
    b32 PreferWindowedFullscreen;
        
    ivec2 GrabbedCoord;
    u32 Resizing;
    SDL_Cursor *HandCursor;
    SDL_Cursor *WestEastCursor;
    SDL_Cursor *NorthSouthCursor;
    SDL_Cursor *SouthEastCursor;
    SDL_Cursor *ArrowCrossCursor;
};

/*-------------*/
/*   ATOMICS   */
/*-------------*/

inline u32 Atomic_Inc(atomic *Val)                   { return SDL_AtomicAdd((SDL_atomic_t *)Val, 1); }
inline u32 Atomic_Dec(atomic *Val)                   { return SDL_AtomicAdd((SDL_atomic_t *)Val,-1); }
inline u32 Atomic_Get(atomic *Val)                   { return SDL_AtomicGet((SDL_atomic_t *)Val); }
inline u32 Atomic_Set(atomic *Val, u32 New)          { return SDL_AtomicSet((SDL_atomic_t *)Val, New); }
inline u32 Atomic_Add(atomic *Val, u32 Add)          { return SDL_AtomicAdd((SDL_atomic_t *)Val, Add); }
inline b32 Atomic_Cas(atomic *Val, u32 New, u32 Old) { return SDL_AtomicCAS((SDL_atomic_t *)Val, Old, New); }

/*-------------*/
/*   THREAD    */
/*-------------*/

inline mutex Mutex_Create()      { return { (mi)SDL_CreateMutex() }; }
inline cond Cond_Create()        { return { (mi)SDL_CreateCond() }; }
inline sem Sem_Create(u32 Value) { return { (mi)SDL_CreateSemaphore(Value) }; }

inline void Mutex_Destroy(mutex Mutex) { SDL_DestroyMutex((SDL_mutex *)Mutex.Handle); }
inline void Cond_Destroy(cond Cond)    { SDL_DestroyCond((SDL_cond *)Cond.Handle); }
inline void Sem_Destroy(sem Sem)       { SDL_DestroySemaphore((SDL_sem *)Sem.Handle); }

inline void Mutex_Lock(mutex Mutex)           { SDL_LockMutex((SDL_mutex *)Mutex.Handle); };
inline void Mutex_Unlock(mutex Mutex)         { SDL_UnlockMutex((SDL_mutex *)Mutex.Handle); };
inline void Cond_Signal(cond Cond)            { SDL_CondSignal((SDL_cond *)Cond.Handle); }
inline void Cond_Wait(cond Cond, mutex Mutex) { SDL_CondWait((SDL_cond *)Cond.Handle, (SDL_mutex *)Mutex.Handle); }
inline void Sem_Post(sem Sem)                 { SDL_SemPost((SDL_sem *)Sem.Handle); }
inline void Sem_Wait(sem Sem)                 { SDL_SemWait((SDL_sem *)Sem.Handle); }
inline b32 Sem_TryWait(sem Sem)               { return SDL_SemTryWait((SDL_sem *)Sem.Handle) == 0; };
inline u32 Sem_Value(sem Sem)                 { return SDL_SemValue((SDL_sem *)Sem.Handle); }

static thread
Thread_Create(thread_func *ThreadFunc, void *Data)
{
#if COMPILE_WINDOWS
    HANDLE Thread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadFunc,Data,0,0);
    return { (mi)Thread };
#else
#error thread creation not implemented for this platform!
#endif
}

static void
Thread_Wait(thread Thread)
{
    WaitForSingleObject((HANDLE)Thread.Handle, INFINITE);
}

/*-------------*/
/*    FILE     */
/*-------------*/

static file
File_OpenExisting(string Filename)
{
    Filename = TString_Null(Filename);
    SDL_RWops *SDLRW = SDL_RWFromFile(Filename.Data, "r+b");
    if (SDLRW) return { (mi)SDLRW };
    return { };
}

static file
File_OpenEmpty(string Filename)
{
    Filename = TString_Null(Filename);
    SDL_RWops *SDLRW = SDL_RWFromFile(Filename.Data, "w+b");
    if (SDLRW) return { (mi)SDLRW };
    return { };
}

static file
File_Open(string Filename)
{
    Filename = TString_Null(Filename);
    file File = File_OpenExisting(Filename);
    if (File_Valid(File)) return File;
    File = File_OpenEmpty(Filename);
    if (File_Valid(File)) return File;
    return { };
}

static void
File_Close(file File)
{
    SDL_RWclose((SDL_RWops *)File.Handle);
}

static b32
File_Valid(file File)
{
    return !!File.Handle;
}

static mi
File_Size(file File)
{
    SDL_RWops *SDLRW = (SDL_RWops *)File.Handle;
    SDL_RWseek(SDLRW, 0, RW_SEEK_SET);
    return SDL_RWseek(SDLRW, 0, RW_SEEK_END);
}

static u8
File_ReadByte(file File, mi Offset)
{
    SDL_RWops *SDLRW = (SDL_RWops *)File.Handle;
    SDL_RWseek(SDLRW, Offset, RW_SEEK_SET);
    return SDL_ReadU8(SDLRW);
}

static void
File_WriteByte(file File, mi Offset, u8 Byte)
{
    SDL_RWops *SDLRW = (SDL_RWops *)File.Handle;
    SDL_RWseek(SDLRW, Offset, RW_SEEK_SET);
    SDL_WriteU8(SDLRW, Byte);
}

static b32
File_ReadData(file File, mi Offset, void *Data, mi Bytes)
{
    SDL_RWops *SDLRW = (SDL_RWops *)File.Handle;
    SDL_RWseek(SDLRW, Offset, RW_SEEK_SET);
    return (b32)SDL_RWread(SDLRW, Data, Bytes, 1);
}

static b32
File_WriteData(file File, mi Offset, const void *Data, mi Bytes)
{
    SDL_RWops *SDLRW = (SDL_RWops *)File.Handle;
    SDL_RWseek(SDLRW, Offset, RW_SEEK_SET);
    return (b32)SDL_RWwrite(SDLRW, Data, Bytes, 1);
}

inline void*
File_ReadEntireFile(string Filename, mi *Bytes)
{
    if (Bytes) *Bytes = 0;
    file File = File_OpenExisting(Filename);
    if (!File_Valid(File)) return 0;
    u64 FileSize = File_Size(File);
    u8 *FileData = (u8 *)Api_Malloc(FileSize);
    if (!File_ReadData(File, 0, FileData, FileSize))
    {
        Api_Free(FileData);
        FileData = 0;
    }
    else if (Bytes)
        *Bytes = FileSize;

    File_Close(File);
    return FileData;
}

inline void
File_WriteEntireFile(string Filename, const void *Data, mi Bytes)
{
    file File = File_OpenEmpty(Filename);
    if (!File_Valid(File)) return;
    File_WriteData(File, 0, Data, Bytes);
    File_Close(File);
}

/*-------------*/
/*     API     */
/*-------------*/

inline void*
Api_Talloc(mi Size)
{
    Assert(TempArena->MarkerCount > 0);
    return Arena_PushSize(TempArena, Size);
}

inline void*
Api_Malloc(mi Size)
{
    return Zone_Malloc(GlobalZone, Size);
}

inline void*
Api_Realloc(void *Ptr, mi Size)
{
    return Zone_Realloc(GlobalZone, Ptr, Size);
}

inline void
Api_Free(void *Ptr)
{
    Zone_Free(GlobalZone, Ptr);
}

inline void
Api_PrintString(u32 Target, string String)
{
    String = TString_Null(String);
    switch (Target)
    {
        case 0: SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, String.Data); break;
        case 1: SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, String.Data); break;
        case 2: SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, String.Data); break;
    }
}

inline u8
Api_GetGamepad()
{
    return GlobalGamepad;
}

/*--------------*/
/*     MAIN     */
/*--------------*/

static void
Main_SDLCleanUp()
{
#if COMPILE_GFX_OPENGL
    SDL_GL_DeleteContext(GlobalGLContext);
    SDL_GL_UnloadLibrary();
#endif
#if COMPILE_GFX_VULKAN
    if ((void *)Vulkan_Destroy)
        Vulkan_Destroy(GlobalVKInstance);
    SDL_Vulkan_UnloadLibrary();
#endif
    SDL_CloseAudioDevice(GlobalAudioDevice);
    SDL_Quit();
}

static i32
Main_Error(const char *Error)
{
    const char *SDL_Error = SDL_GetError();
    if (SDL_Error && *SDL_Error)
        Api_Error(TString(SDL_Error));

    Api_Error(TString(Error));
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Platform Error!", Error, 0);
    
    Main_SDLCleanUp();
    GlobalRunning = false;
    return 1;
}

inline u64
Main_GetWallClock()
{
    return SDL_GetPerformanceCounter();
}

inline f32
Main_GetSecondsElapsed(u64 Start, u64 End = Main_GetWallClock())
{
    f32 Result = ((f32)(End - Start) / (f32)GlobalPerfCountFrequency);
    return Result;
}

inline u64
Main_GetMillisecondsElapsed(u64 Start, u64 End)
{
    u64 Result = ((End - Start) * 1000) / GlobalPerfCountFrequency;
    return Result;
}

/*--------------*/
/*     NES      */
/*--------------*/

static i32
NES_Thread(void *Data)
{
    f64 TargetTimePerUpdate = GlobalPerfCountFrequency / (f64)CPU_HZ;
    f64 LastTime = (f64)Main_GetWallClock();

    while (GlobalRunning)
    {
        Console_Step(GlobalConsole);

        /* TIMING */
        f64 TargetTime = LastTime + TargetTimePerUpdate;
        f64 CurrentTime = (f64)Main_GetWallClock();
        while (CurrentTime < TargetTime)
            CurrentTime = (f64)Main_GetWallClock();
        LastTime = TargetTime;
    }

    return 1;
}

static void
NES_Pause(main_state *MainState)
{
    if (!GlobalConsole) return;
    if (!MainState->NESThread.Handle) return;

    SDL_ClearQueuedAudio(GlobalAudioDevice);
    GlobalRunning = false;
    Thread_Wait(MainState->NESThread);
    MainState->NESThread.Handle = 0;
    GlobalRunning = true;
}

static void
NES_Resume(main_state *MainState)
{
    if (!GlobalConsole) return;
    if (MainState->NESThread.Handle) return;

    SDL_ClearQueuedAudio(GlobalAudioDevice);
    MainState->NESThread = Thread_Create(NES_Thread, 0);
}

static void
NES_Reset(main_state *MainState)
{
    NES_Pause(MainState);
    if (GlobalConsole)
        Console_Reset(GlobalConsole);
    NES_Resume(MainState);
}

static void
NES_Power(main_state *MainState)
{
    NES_Pause(MainState);
    if (GlobalConsole)
        Console_Power(GlobalConsole);
    NES_Resume(MainState);
}

static void
NES_Create(main_state *MainState, string Filename)
{
    string Savename = Filename;
    Savename.Length -= 4;
    Savename = String_SplitRight(&Savename, '\\');
    Savename = TString_Concat(PrefPath, Savename, S(".sav"));
    cart *Cart = iNes_Load(Filename, Savename);
    if (!Cart) return;
    console *Console = Console_Create(Cart);
    if (Console)
    {
        if (MainState->NESThread.Handle)
        {
            GlobalRunning = false;
            Thread_Wait(MainState->NESThread);
            GlobalRunning = true;
        }
        if (GlobalConsole) Console_Free(GlobalConsole);
        GlobalConsole = Console;
        MainState->NESThread = Thread_Create(NES_Thread, 0);
    }
}

/*--------------*/
/*    WINDOW    */
/*--------------*/

static ivec2
Main_GetWindowSize(SDL_Window *Window)
{
    i32 Width, Height;
#if COMPILE_GFX_OPENGL
    SDL_GL_GetDrawableSize(Window, &Width, &Height);
#elif COMPILE_GFX_VULKAN
    SDL_Vulkan_GetDrawableSize(Window, &Width, &Height);
#endif
    ivec2 WindowDimension = { Width, Height };
    return WindowDimension;
}

static b32
Main_SetExclusiveFullscreen(SDL_Window *Window)
{
    i32 DiplayIndex = SDL_GetWindowDisplayIndex(Window);
    if (DiplayIndex < 0) return false;

    SDL_DisplayMode DisplayMode = { };
    if (SDL_GetCurrentDisplayMode(DiplayIndex, &DisplayMode))
        return false;

    if (SDL_SetWindowDisplayMode(Window, &DisplayMode))
        return false;

    if (SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN))
        return false;

    return true;
}

static void
Main_ToggleFullscreen(main_state *MainState, SDL_Window *Window)
{    
    u32 Fullscreen = (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (SDL_GetWindowFlags(Window) & Fullscreen)
    {
        SDL_SetWindowFullscreen(Window, 0);
        return;
    }

    if(MainState->PreferWindowedFullscreen)
    {
        SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else if (!Main_SetExclusiveFullscreen(Window))
    {
        MainState->PreferWindowedFullscreen = true;
        SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

static void
Main_SetVSync(main_state *MainState, b32 VSync)
{
    if (MainState->VSyncState == VSync)
        return;

    MainState->VSyncState = VSync;
    if (VSync) SDL_GL_SetSwapInterval(1);
    else       SDL_GL_SetSwapInterval(0);
}

#if COMPILE_GFX_OPENGL
static SDL_Window *
Main_CreateOpenGLWindow()
{
    ivec2 Resolution = iVec2(300, 224);
    ivec2 WindowDim = Resolution * iVec2(3);
    // WindowDim = iVec2(1920, 1080);

#if (COMPILE_PI || COMPILE_EMSCRIPTEN)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                        SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
                        SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    u32 WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;
    SDL_Window *Window = SDL_CreateWindow(GAME_NAME,
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED,
                                          WindowDim.x, 
                                          WindowDim.y,
                                          WindowFlags);
    if (!Window)
    {
        Main_Error("Couldn't create Window!");
        return 0;
    }

    SDL_SetWindowMinimumSize(Window, Resolution.x, Resolution.y);

    GlobalGLContext = SDL_GL_CreateContext(Window);
    if (!GlobalGLContext)
    {
        Main_Error("Couldn't create OpenGL Context!");
        return 0;
    }
    if (!GL_LoadFunctions())
    {
        Main_Error("Couldn't load OpenGL functions!");
        return 0;
    }
    if (OpenGL_Init())
    {
        Main_Error("Couldn't set up OpenGL state!");
        return 0;
    }

    return Window;
}
#endif

#if COMPILE_GFX_VULKAN
static SDL_Window *
Main_CreateVulkanWindow(iv2 Resolution)
{
    iv2 WindowDim = Resolution * iV2(3);
    u32 WindowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;
    SDL_Window *Window = SDL_CreateWindow(GAME_NAME,
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED,
                                          WindowDim.x, 
                                          WindowDim.y,
                                          WindowFlags);
    if (!Window)
    {
        Main_Error("Couldn't create Window!");
        return 0;
    }

    SDL_SetWindowMinimumSize(Window, Resolution.x, Resolution.y);

    vkGetInstanceProcAddr = 
        (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
    u32 ExtCount;
    if (!SDL_Vulkan_GetInstanceExtensions(Window, &ExtCount, 0))
    {
        Main_Error("Couldn't query Vulkan required extensions!");
        return 0;
    }
    const char** ExtNames = (const char **)Api_Malloc(sizeof(char *) * (ExtCount + 1));
    if (!SDL_Vulkan_GetInstanceExtensions(Window, &ExtCount, ExtNames))
    {
        Api_Free(ExtNames);
        Main_Error("Couldn't query Vulkan extension names!");
        return 0;
    }
#if COMPILE_DEV
    ExtNames[ExtCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
#endif
    b32 Success = Vulkan_Instance(&GlobalVKInstance, ExtCount, ExtNames);
    Api_Free(ExtNames);
    if (!Success)
    {
        Main_Error("Couldn't create Vulkan instance!");
        return 0;
    }

    VkSurfaceKHR Surface;
    if (!SDL_Vulkan_CreateSurface(Window, GlobalVKInstance, &Surface))
    {
        Main_Error("Couldn't create Vulkan surface!");
        return 0;
    }

    if (!Vulkan_Init(GlobalVKInstance, Surface))
    {
        Main_Error("Couldn't init Vulkan!");
        return 0;
    }

    return Window;
}
#endif

static u32
Main_GetResizeRegion(SDL_Window *Window, ivec2 Coord)
{
    ivec2 Dim;
    SDL_GetWindowSize(Window, &Dim.x, &Dim.y);
    u32 Result = 0;
    if (Dim.x - Coord.x < 32) Result |= 1;
    if (Dim.y - Coord.y < 32) Result |= 2;
    return Result;
}

static void
Main_SetRegionCursor(u32 Region, main_state *MainState)
{
    SDL_Cursor *Cursor;
    switch (Region)
    {
        case 3: Cursor = MainState->SouthEastCursor; break;
        case 2: Cursor = MainState->NorthSouthCursor; break;
        case 1: Cursor = MainState->WestEastCursor; break;
        default: Cursor = MainState->HandCursor; break;
    }
    SDL_SetCursor(Cursor);
}

static void
Main_CollectEvents(SDL_Window *Window, main_state *MainState)
{
    b32 StartTextInput = false;

    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
        /* PROCESS EVENTS */
        switch (Event.type)
        {
            /* QUIT EVENT */
            case SDL_QUIT:
            {
                GlobalRunning = false;
            } break;

            /* KEY EVENTS */
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                b32 IsDown = (Event.key.state == SDL_PRESSED);
                u32 ScanCode = Event.key.keysym.scancode;
                u32 KeyCode = Event.key.keysym.sym;

                if (IsDown)
                {
                    if (Event.key.repeat)
                        break;

                    if (ScanCode == SDL_SCANCODE_ESCAPE)
                    {
                        GlobalRunning = false;
                        break;
                    }

                    if (ScanCode == SDL_SCANCODE_R)
                    {
                        NES_Reset(MainState);
                        break;
                    }

                    b32 AltKeyDown = !!(Event.key.keysym.mod & KMOD_ALT);
                    if (AltKeyDown)
                    {
                        if (ScanCode == SDL_SCANCODE_F4)
                        {
                            GlobalRunning = false;
                            break;
                        }

                        if (ScanCode == SDL_SCANCODE_RETURN && Window)
                        {
                            Main_ToggleFullscreen(MainState, Window);
                            break;
                        }
                    }
                }

                if (Event.key.repeat)
                    break;
                
                SDL_ShowCursor(SDL_DISABLE);
                Input_Key(&MainState->Input, KeyCode, IsDown);
            } break;

            /* MOUSE EVENTS */
            case SDL_MOUSEBUTTONDOWN:
            {
                MainState->GrabbedCoord = iVec2(Event.button.x, Event.button.y);
                SDL_CaptureMouse(SDL_TRUE);
                MainState->Resizing = Main_GetResizeRegion(Window, MainState->GrabbedCoord);
                if (!MainState->Resizing)
                    SDL_SetCursor(MainState->ArrowCrossCursor);
            } break;

            case SDL_MOUSEBUTTONUP:
            {
                MainState->GrabbedCoord = iVec2(-1);
                SDL_CaptureMouse(SDL_FALSE);
                if (!MainState->Resizing)
                    SDL_SetCursor(MainState->HandCursor);
                MainState->Resizing = 0;
            } break;

            case SDL_MOUSEMOTION:
            {
                SDL_ShowCursor(SDL_ENABLE);
                if (MainState->GrabbedCoord.x != -1)
                {
                    ivec2 Pos;
                    SDL_GetWindowPosition(Window, &Pos.x, &Pos.y);
                    ivec2 Delta = iVec2(Event.motion.x, Event.motion.y) - MainState->GrabbedCoord;

                    if (MainState->Resizing)
                    {
                        ivec2 Dim;
                        SDL_GetWindowSize(Window, &Dim.x, &Dim.y);
                        if (MainState->Resizing & 1)
                        {
                            Dim.x += Delta.x;
                            if (Dim.x >= 256)
                                MainState->GrabbedCoord.x += Delta.x;
                        }
                        if (MainState->Resizing & 2)
                        {
                            Dim.y += Delta.y;
                            if (Dim.y >= 224)
                                MainState->GrabbedCoord.y += Delta.y;
                        }
                        SDL_SetWindowSize(Window, Dim.x, Dim.y);
                    }
                    else // Moving
                    {
                        Pos += Delta;
                        SDL_SetWindowPosition(Window, Pos.x, Pos.y);
                    }
                }
                else
                {
                    u32 Region = Main_GetResizeRegion(Window, iVec2(Event.motion.x, Event.motion.y));
                    Main_SetRegionCursor(Region, MainState);
                }
            } break;

            /* DRAG AND DROP */
            case SDL_DROPFILE:
            {
                string Filename = String(Event.drop.file);
                if (String_EndsWith(Filename, S(".nes")) ||
                    String_EndsWith(Filename, S(".fc")))
                {
                    NES_Create(MainState, Filename);
                }
                else if (String_EndsWith(Filename, S(".pal")))
                {
                    OpenGL_LoadPalette(Filename);
                }
                else if (String_EndsWith(Filename, S(".fm2")))
                {
                    gamepad_playback *Playback = Input_LoadFM2(Filename);
                    if (Playback)
                    {
                        NES_Pause(MainState);
                        Gamepad_SetPlayback(GlobalConsole->Gamepad, Playback);
                        if (GlobalConsole)
                            Console_Power(GlobalConsole);
                        NES_Resume(MainState);
                    }
                }
                SDL_free(Event.drop.file);
            } break;

            /* WINDOW EVENTS */
            case SDL_WINDOWEVENT:
            {
                switch (Event.window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    {
                        // SDL_PauseAudioDevice(GlobalAudioDevice, 0);
                    } break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        // SDL_PauseAudioDevice(GlobalAudioDevice, 1);
                    } break;

                    case SDL_WINDOWEVENT_ENTER:
                    {
                        SDL_SetCursor(MainState->HandCursor);
                    } break;

                    case SDL_WINDOWEVENT_LEAVE:
                    {
                        SDL_SetCursor(0);
                    } break;

                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        f32 AR = Event.window.data2 / (f32)Event.window.data1;
                    } break;
                }
            } break;

            /* GAMEPAD EVENTS */
            case SDL_CONTROLLERDEVICEADDED:
            {
                SDL_GameControllerOpen(Event.cdevice.which);
                SDL_GameControllerEventState(SDL_ENABLE);
            } break;

            case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_GameController *Gamepad = SDL_GameControllerFromInstanceID(Event.cdevice.which);
                SDL_GameControllerClose(Gamepad);
            } break;

            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
            {
                SDL_ShowCursor(SDL_DISABLE);
                u8 Button = Event.cbutton.button;
                b32 State = Event.cbutton.state == SDL_PRESSED;
                Input_Button(&MainState->Input, Button, State);
            } break;

            case SDL_CONTROLLERAXISMOTION:
            {
                SDL_ShowCursor(SDL_DISABLE);
                u8 Axis = Event.caxis.axis;
                f32 State = Event.caxis.value;
                State = CLAMP(State * (1.0f / 32767.0f), -1.0f, 1.0f);
                Input_Axis(&MainState->Input, Axis, State);
            } break;
        }
    }
}

static b32
Main_CheckCPUFeatures()
{
    b32 AllFeatures = true;

#if COMPILE_DEV
    if (!CPUID.HasRDTSC)
    {
        Api_Error(S("RDTSC is not supported on this CPU!"));
        AllFeatures = false;
    }
#endif
    
#if COMPILE_X64
    if (!CPUID.HasSSE)
    {
        Api_Error(S("SSE is not supported on this CPU!"));
        AllFeatures = false;
    }
    if (!CPUID.HasSSE2)
    {
        Api_Error(S("SSE2 is not supported on this CPU!"));
        AllFeatures = false;
    }
    if (!CPUID.HasSSE3)
    {
        Api_Error(S("SSE3 is not supported on this CPU!"));
        AllFeatures = false;
    }

    #if COMPILE_SSE
        if (!CPUID.HasSSSE3)
        {
            Api_Error(S("SSSE3 is not supported on this CPU!"));
            AllFeatures = false;
        }
        if (!CPUID.HasSSE41)
        {
            Api_Error(S("SSE4.1 is not supported on this CPU!"));
            AllFeatures = false;
        }
        if (!CPUID.HasSSE42)
        {
            Api_Error(S("SSE4.2 is not supported on this CPU!"));
            AllFeatures = false;
        }
        if (!CPUID.HasFMA)
        {
            Api_Error(S("FMA is not supported on this CPU!"));
            AllFeatures = false;
        }
    #endif
#endif

#if COMPILE_NEON
    if (!SDL_HasNEON())
    {
        Api_Error(S("NEON is not supported on this CPU!"));
        AllFeatures = false;
    }
#endif
    
    return AllFeatures;
}

int SDL_main(int argc, char **argv)
{
    /* INIT */
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER))
        return Main_Error("Couldn't initialize SDL!");
    
    main_state MainState = { };
    MainState.GrabbedCoord = iVec2(-1);
    MainState.HandCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    MainState.WestEastCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    MainState.NorthSouthCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    MainState.SouthEastCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    MainState.ArrowCrossCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);

    /* GLOBALS */
    GlobalPerfCountFrequency = SDL_GetPerformanceFrequency();
    GlobalZone = Zone_Clear((memory_zone *)SDL_malloc(Megabytes(8)), Megabytes(8));
    TempArena = Arena_Clear((memory_arena *)SDL_malloc(Kilobytes(1)), Kilobytes(1));
    memory_arena_marker InitMarker = Arena_PlaceArenaMarker(TempArena);

    BasePath = String(SDL_GetBasePath());
    PrefPath = String(SDL_GetPrefPath(ORG_NAME,GAME_NAME));

    /* CHECK CPU FEATURES */
    if (!Main_CheckCPUFeatures())
        return Main_Error("Missing CPU features!");
    
    /* START TIMING */
    u64 StartTime = Main_GetWallClock();

    /* AUDIO */
    {
#if 0
        //NOTE: Use directsound on windows maybe
        if (SDL_AudioInit("directsound")) 
            return Main_Error("Couldn't initialize Directsound!");
#endif
        SDL_AudioSpec OpenedSpec, DesiredSpec = { };
        DesiredSpec.freq = APU_DAC_HZ;
        DesiredSpec.format = AUDIO_F32LSB;
        DesiredSpec.channels = 1;
        DesiredSpec.samples = 4096;
        DesiredSpec.callback = 0;
        DesiredSpec.userdata = 0;

        GlobalAudioDevice = SDL_OpenAudioDevice(0, 0, &DesiredSpec, &OpenedSpec, 0);
        if (!GlobalAudioDevice) Api_Error(S("Couldn't open audio device!"));
        SDL_PauseAudioDevice(GlobalAudioDevice, 0);
    }

    /* GRAPHICS */
#if COMPILE_GFX_OPENGL
    SDL_Window *Window = Main_CreateOpenGLWindow();
#elif COMPILE_GFX_VULKAN
    SDL_Window *Window = Main_CreateVulkanWindow();
#endif
    if (!Window) return Main_Error("Couldn't create window!");

    SDL_ShowWindow(Window);
    SDL_RaiseWindow(Window);
    SDL_DisableScreenSaver();

    /* INPUT */
    Input_Init(&MainState.Input);
    SDL_GameControllerAddMapping(GamePadDB);
    SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
    SDL_StopTextInput();

    // SDL_GL_SetSwapInterval(0);

    Arena_RevertToArenaMarker(InitMarker);

    /* GAME LOOP */
    u64 LastTime = Main_GetWallClock();
    while (GlobalRunning)
    {
        ARENA_STACK_MARKER(TempArena);

        Main_CollectEvents(Window, &MainState);
#if !(OPENGL_USETEXTUREBUFFER)
        if (Atomic_Set(&GlobalScreenChanged, 0))
            OpenGL_Frame((u8 *)GlobalScreen);
#endif
        OpenGL_Blit(Main_GetWindowSize(Window));
        SDL_GL_SwapWindow(Window);
        SDL_Delay(1);

        /* FRAME TIMING */
        u64 EndTime = Main_GetWallClock();
        LastTime = EndTime;
    }

    if (GlobalConsole)
    {
        Thread_Wait(MainState.NESThread);
        Console_Free(GlobalConsole);
    }
    Main_SDLCleanUp();

    return 0;
}
