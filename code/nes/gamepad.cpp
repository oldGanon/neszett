global u8 Strobe = 0;
global u8 GamepadReadSerial[2] = { };

inline u8
Gamepad_Read(u8 Index)
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
Gamepad_Write(u8 Value)
{
	Strobe = Value & 1;
	if (Strobe)
	{
		GamepadReadSerial[0] = 0;
		GamepadReadSerial[1] = 0;
	}
}
