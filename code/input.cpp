enum input_nes_buttons
{
    NES_BUTTON_A      = 0,
    NES_BUTTON_B      = 1,
    NES_BUTTON_SELECT = 2,
    NES_BUTTON_START  = 3,
    NES_BUTTON_UP     = 4,
    NES_BUTTON_DOWN   = 5,
    NES_BUTTON_LEFT   = 6,
    NES_BUTTON_RIGHT  = 7,
    NES_BUTTON_COUNT,
};

struct input
{
    u32 Buttons[NES_BUTTON_COUNT];
};

static void
Input_Key(input *Input, u32 KeyCode, b32 Down)
{
    for (u8 i = 0; i < NES_BUTTON_COUNT; ++i)
    {
        if (KeyCode != Input->Buttons[i])
            continue;

        u8 Mask = 1 << (i & 7);
        if (Down)
            GlobalGamepad |= Mask;
        else
            GlobalGamepad &= ~Mask;
    }
}

static void
Input_Button(input *Input, u32 Button, b32 Down)
{
    u8 NesButton;
    switch (Button)
    {
        case SDL_CONTROLLER_BUTTON_A:             NesButton = NES_BUTTON_A; break;
        case SDL_CONTROLLER_BUTTON_B:             NesButton = NES_BUTTON_B; break;
        case SDL_CONTROLLER_BUTTON_X:             NesButton = NES_BUTTON_B; break;
        case SDL_CONTROLLER_BUTTON_Y:             NesButton = NES_BUTTON_A; break;
        case SDL_CONTROLLER_BUTTON_BACK:          NesButton = NES_BUTTON_SELECT; break;
        case SDL_CONTROLLER_BUTTON_GUIDE:         return;
        case SDL_CONTROLLER_BUTTON_START:         NesButton = NES_BUTTON_START; break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:     return;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:    return;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:       NesButton = NES_BUTTON_UP; break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     NesButton = NES_BUTTON_DOWN; break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     NesButton = NES_BUTTON_LEFT; break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    NesButton = NES_BUTTON_RIGHT; break;
        default: return;
    }

    u8 Mask = 1 << (NesButton & 7);
    if (Down)
        GlobalGamepad |= Mask;
    else
        GlobalGamepad &= ~Mask;
}

static void
Input_Axis(input *Input, u32 Axis, f32 State)
{
    if (Axis == SDL_CONTROLLER_AXIS_LEFTX)
    {
        f32 Deadzone = 0.25f;
        if (State > Deadzone)
        {
            GlobalGamepad |= 0x80;
            GlobalGamepad &= ~0x40;
        }
        else if (State < -Deadzone)
        {
            GlobalGamepad |= 0x40;
            GlobalGamepad &= ~0x80;
        }
        else GlobalGamepad &= ~0xC0;
    }
    else if (Axis == SDL_CONTROLLER_AXIS_LEFTY)
    {
        f32 Deadzone = 0.5f;
        if (State > Deadzone)
        {
            GlobalGamepad |= 0x20;
            GlobalGamepad &= ~0x10;
        }
        else if (State < -Deadzone)
        {
            GlobalGamepad |= 0x10;
            GlobalGamepad &= ~0x20;
        }
        else GlobalGamepad &= ~0x30;
    }
}

static u8
Input_NESButtonFromName(string String)
{
    if (String == S("A"))      return NES_BUTTON_A;
    if (String == S("B"))      return NES_BUTTON_B;
    if (String == S("SELECT")) return NES_BUTTON_SELECT;
    if (String == S("START"))  return NES_BUTTON_START;
    if (String == S("UP"))     return NES_BUTTON_UP;
    if (String == S("DOWN"))   return NES_BUTTON_DOWN;
    if (String == S("LEFT"))   return NES_BUTTON_LEFT;
    if (String == S("RIGHT"))  return NES_BUTTON_RIGHT;
    return 0xFF;
}

static string
Input_NESButtonToName(u8 Button)
{
    switch (Button & 7)
    {
        case NES_BUTTON_A:      return S("A");
        case NES_BUTTON_B:      return S("B");
        case NES_BUTTON_SELECT: return S("SELECT");
        case NES_BUTTON_START:  return S("START");
        case NES_BUTTON_UP:     return S("UP");
        case NES_BUTTON_DOWN:   return S("DOWN");
        case NES_BUTTON_LEFT:   return S("LEFT");
        case NES_BUTTON_RIGHT:  return S("RIGHT");
        default: return String();
    }
}

static void
Input_WriteControlsTXT(input *Input)
{
    string File = String();
    for (u8 i = 0; i < NES_BUTTON_COUNT; ++i)
    {
        string Key = String(SDL_GetKeyName(Input->Buttons[i]));
        string Button = Input_NESButtonToName(i);
        File = TString_Concat(File, Key, S(" := "), Button, S("\r\n"));
    }
    File_WriteEntireFile(S("controls.txt"), File.Data, File.Length);
}

static void
Input_Init(input *Input)
{
    Input->Buttons[NES_BUTTON_A]      = SDLK_DOWN;
    Input->Buttons[NES_BUTTON_B]      = SDLK_LEFT;
    Input->Buttons[NES_BUTTON_SELECT] = SDLK_RSHIFT;
    Input->Buttons[NES_BUTTON_START]  = SDLK_RETURN;
    Input->Buttons[NES_BUTTON_UP]     = SDLK_w;
    Input->Buttons[NES_BUTTON_LEFT]   = SDLK_a;
    Input->Buttons[NES_BUTTON_DOWN]   = SDLK_s;
    Input->Buttons[NES_BUTTON_RIGHT]  = SDLK_d;

    mi Length;
    void *Data = File_ReadEntireFile(S("controls.txt"), &Length);
    if (!Data) return Input_WriteControlsTXT(Input);

    string File = String((char *)Data, Length);

    while (File.Length)
    {
        string Line = String_SplitLeft(&File, '\n');
        if (String_EndsWith(Line, S("\r"))) Line.Length--;

        string Keyname = UTF8_Trim(String_SplitLeft(&Line, S(":=")));
        if (!Keyname.Length) continue;
        string Key = UTF8_Trim(Line);
        if (!Key.Length) continue;

        Keyname = TString_Null(Keyname);
        u32 Code = SDL_GetKeyFromName(Keyname.Data);
        if (Code == SDLK_UNKNOWN) continue;
        u8 NesButton = Input_NESButtonFromName(Key);
        if (NesButton == 0xFF) continue;
        Input->Buttons[NesButton] = Code;
    }

    Api_Free(Data);
}

static gamepad_playback*
Input_LoadFM2(string Filename)
{
    mi Length;
    void *Data = File_ReadEntireFile(Filename, &Length);
    if (!Data) return 0;

    string File = String((char *)Data, Length);

    gamepad_playback *Result = 0;

    b32 Binary = 0;
    while (!String_StartsWith(File, S("|")))
    {
        string Line = String_SplitLeft(&File, '\n');
        string Key = String_SplitLeft(&Line, ' ');

        if (Key == S("version"))
        {
            if (!String_StartsWith(Line, S("3")))
            {
                Api_Free(Data);
                return 0;
            }
        }
        // else if (Key == S("emuVersion"));
        // else if (Key == S("rerecordCount"));
        // else if (Key == S("palFlag"));
        // else if (Key == S("NewPPU"));
        // else if (Key == S("FDS"));
        // else if (Key == S("fourscore"));
        // else if (Key == S("port0"));
        // else if (Key == S("port1"));
        // else if (Key == S("port2"));
        else if (Key == S("binary"))
        {
            if (String_StartsWith(Line, S("true")))
                Binary = true;
        }
        // else if (Key == S("length"));
        // else if (Key == S("romFilename"));
        // else if (Key == S("comment"));
        // else if (Key == S("subtitle"));
        // else if (Key == S("guid"));
        // else if (Key == S("romChecksum"));
        // else if (Key == S("savestate"));
    }

    if (Binary)
    {

    }
    else
    {
        u32 FrameCount = 0;
        string InputLog = File;
        while (InputLog.Length)
        {
            ++FrameCount;
            string Line = String_SplitLeft(&InputLog, '\n');
        }

        Result = (gamepad_playback *)Api_Malloc(FrameCount * sizeof(gamepad_frame) + 8);
        Result->Size = FrameCount;
        Result->Index = 1;
        gamepad_frame *Frame = Result->Frames;
        InputLog = File;
        while (InputLog.Length)
        {
            string Line = String_SplitLeft(&InputLog, '\n');
            String_SplitLeft(&Line, '|');

            *Frame = { };

            string Flags = String_SplitLeft(&Line, '|');
            switch (Flags.Data[0])
            {
                case '3': Frame->Flags = 3; break;
                case '2': Frame->Flags = 2; break;
                case '1': Frame->Flags = 1; break;
            }

            string Gamepad0 = String_SplitLeft(&Line, '|');
            if (Gamepad0.Length == 8)
            for (u32 i = 0; i < 8; ++i)
                if (Gamepad0.Data[i] != ' ' && Gamepad0.Data[i] != '.')
                    Frame->Gamepad[0] |= 0x80 >> i;

            string Gamepad1 = String_SplitLeft(&Line, '|');
            if (Gamepad1.Length == 8)
            for (u32 i = 0; i < 8; ++i)
                if (Gamepad1.Data[i] != ' ' && Gamepad1.Data[i] != '.')
                    Frame->Gamepad[1] |= 0x80 >> i;

            ++Frame;
        }
    }

    Api_Free(Data);

    return Result;
}
