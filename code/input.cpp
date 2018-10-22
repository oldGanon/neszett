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

struct input_key
{
    u32 KeyCode;
};

struct input
{
    input_key Buttons[NES_BUTTON_COUNT];
};

static void
Input_Key(input *Input, u32 KeyCode, b32 Down)
{
    for (u8 i = 0; i < NES_BUTTON_COUNT; ++i)
    {
        if (KeyCode != Input->Buttons[i].KeyCode)
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
        string Key = String(SDL_GetKeyName(Input->Buttons[i].KeyCode));
        string Button = Input_NESButtonToName(i);
        File = TString_Concat(File, Key, S(" := "), Button, S("\r\n"));
    }
    File_WriteEntireFile(S("controls.txt"), File.Data, File.Length);
}

static void
Input_Init(input *Input)
{
    Input->Buttons[NES_BUTTON_A].KeyCode      = SDLK_DOWN;
    Input->Buttons[NES_BUTTON_B].KeyCode      = SDLK_LEFT;
    Input->Buttons[NES_BUTTON_SELECT].KeyCode = SDLK_RSHIFT;
    Input->Buttons[NES_BUTTON_START].KeyCode  = SDLK_RETURN;
    Input->Buttons[NES_BUTTON_UP].KeyCode     = SDLK_w;
    Input->Buttons[NES_BUTTON_LEFT].KeyCode   = SDLK_a;
    Input->Buttons[NES_BUTTON_DOWN].KeyCode   = SDLK_s;
    Input->Buttons[NES_BUTTON_RIGHT].KeyCode  = SDLK_d;

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
        if (Code > 255) continue;
        u8 NesButton = Input_NESButtonFromName(Key);
        if (NesButton == 0xFF) continue;
        Input->Buttons[NesButton].KeyCode = Code;
    }

    Api_Free(Data);
}
