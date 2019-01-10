
struct gamepad_frame
{
    u8 Gamepad[2];
    u8 Flags;
};

struct gamepad_playback
{
    u32 Size;
    u32 Index;
    gamepad_frame Frames[1];
};

struct gamepad
{
    console *Console;
    gamepad_playback *Playback;

    u8 Strobe;
    u8 ReadSerial[2];
};

inline u8
Gamepad_GetPlaybackButtons(gamepad_playback *Playback, u8 Index)
{
    return Playback->Frames[Playback->Index].Gamepad[Index];
}

inline u8
Gamepad_Read(gamepad *Gamepad, u8 Index)
{
    u8 Buttons;
    if (Gamepad->Playback)
        Buttons = Gamepad_GetPlaybackButtons(Gamepad->Playback, Index);
    else
    {
        if (Index == 0)
            Buttons = Api_GetGamepad();
        else
            Buttons = 0;
    }

    if (Gamepad->Strobe) Gamepad->ReadSerial[Index] = 0;
    u8 Result = Buttons >> Gamepad->ReadSerial[Index]++;
    if (Gamepad->ReadSerial[Index] > 8)
    {
        Gamepad->ReadSerial[Index] = 8;
        return 1;
    }
    return Result & 1;
}

inline void
Gamepad_Write(gamepad *Gamepad, u8 Value)
{
    Gamepad->Strobe = Value & 1;
    if (Gamepad->Strobe)
    {
        Gamepad->ReadSerial[0] = 0;
        Gamepad->ReadSerial[1] = 0;
    }
}

inline void
Gamepad_NextFrame(gamepad *Gamepad)
{
    if (!Gamepad->Playback) return;
    gamepad_playback *Playback = Gamepad->Playback;
    if (++Playback->Index > Playback->Size)
    {
        Api_Free(Playback);
        Gamepad->Playback = 0;
        return;
    }
}

inline void
Gamepad_SetPlayback(gamepad *Gamepad, gamepad_playback *Playback)
{
    if (Gamepad->Playback)
        Api_Free(Gamepad->Playback);
    Gamepad->Playback = Playback;
}

inline void
Gamepad_Step(gamepad *Gamepad)
{
    if (!Gamepad->Playback) return;
    gamepad_playback *Playback = Gamepad->Playback;
    u8 Flags = Playback->Frames[Playback->Index].Flags;
    if (Flags & 1)
    {
        Gamepad_NextFrame(Gamepad);
        Console_Reset(Gamepad->Console);
    }
    if (Flags & 2)
    {
        Gamepad_NextFrame(Gamepad);
        Console_Power(Gamepad->Console);
    }
}

inline gamepad*
Gamepad_Create(console *Console)
{
    gamepad *Gamepad = (gamepad *)Api_Malloc(sizeof(gamepad));
    Gamepad->Console = Console;
    return Gamepad;
}

inline void
Gamepad_Free(gamepad *Gamepad)
{
    if (Gamepad->Playback)
        Api_Free(Gamepad->Playback);
    Api_Free(Gamepad);
}
