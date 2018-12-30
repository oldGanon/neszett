global u8 Strobe = 0;
global u8 GamepadReadSerial[2] = { };

struct gamepad_playback
{
	u32 Size;
	u32 Index;
	u8 States[1];
};

struct gamepad
{
	u8 Strobe;
	u8 ReadSerial[2];

	gamepad_playback *Playback;
};

inline u8
Gamepad_Read(gamepad *Gamepad, u8 Index)
{
	u8 Buttons = Api_GetGamepad();
	if (Strobe) GamepadReadSerial[Index] = 0;
	u8 Result = Buttons >> GamepadReadSerial[Index]++;
	if (GamepadReadSerial[Index] > 8)
	{
		GamepadReadSerial[Index] = 8;
		return 1;
	}
	return Result & 1;
}

inline void
Gamepad_Write(gamepad *Gamepad, u8 Value)
{
	Strobe = Value & 1;
	if (Strobe)
	{
		GamepadReadSerial[0] = 0;
		GamepadReadSerial[1] = 0;
	}
}

inline gamepad*
Gamepad_Create()
{
	return (gamepad *)Api_Malloc(sizeof(gamepad));
}

inline void
Gamepad_Free(gamepad *Gamepad)
{
	Api_Free(Gamepad);
}
