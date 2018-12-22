struct cpu;
struct ppu;
struct apu;
struct cart;
struct gamepad;
struct console
{
    cpu *CPU;
    ppu *PPU;
    apu *APU;
    cart *Cart;
    gamepad *Gamepad;
};

enum irq_source
{
    IRQ_SOURCE_APU    = 0x01,
    IRQ_SOURCE_MAPPER = 0x02,
};

inline u8 Gamepad_Read(console *Console, u8 Index);
inline void Gamepad_Write(console *Console, u8 Value);
inline u8 Cart_Read(console *Console, u16 Address);
inline void Cart_Write(console *Console, u16 Address, u8 Value);
inline u8 PPU_ReadRegister(console *Console, u16 Address);
inline void PPU_WriteRegister(console *Console, u16 Address, u8 Value);
inline u8 APU_ReadRegister(console *Console, u16 Address);
inline void APU_WriteRegister(console *Console, u16 Address, u8 Value);
inline void Console_TriggerNMI(console *Console);
inline void Console_SetIRQ(console *Console, irq_source Source);
inline void Console_ClearIRQ(console *Console, irq_source Source);

#include "nes/gamepad.cpp"
#include "nes/cart.cpp"
#include "nes/file.cpp"
#include "nes/ppu.cpp"
#include "nes/cpu.cpp"
#include "nes/apu.cpp"

inline u8
Gamepad_Read(console *Console, u8 Index)
{
    return Gamepad_Read(Console->Gamepad, Index);
}

inline void
Gamepad_Write(console *Console, u8 Value)
{
    Gamepad_Write(Console->Gamepad, Value);
}

inline u8
Cart_Read(console *Console, u16 Address)
{
    return Cart_Read(Console->Cart, Address);
}

inline void
Cart_Write(console *Console, u16 Address, u8 Value)
{
    Cart_Write(Console->Cart, Address, Value);
}

inline u8
PPU_ReadRegister(console *Console, u16 Address)
{
    return PPU_ReadRegister(Console->PPU, Address);
}

inline void
PPU_WriteRegister(console *Console, u16 Address, u8 Value)
{
    PPU_WriteRegister(Console->PPU, Address, Value);
}

inline u8
APU_ReadRegister(console *Console, u16 Address)
{
    return APU_ReadRegister(Console->APU, Address);
}

inline void
APU_WriteRegister(console *Console, u16 Address, u8 Value)
{
    APU_WriteRegister(Console->APU, Address, Value);
}

inline void
Console_TriggerNMI(console *Console)
{
    Console->CPU->NMIOccurred = true;
}

inline void
Console_SetIRQ(console *Console, irq_source Source)
{
    Console->CPU->IRQOccurred |= Source;
}

inline void
Console_ClearIRQ(console *Console, irq_source Source)
{
    Console->CPU->IRQOccurred &= ~Source;
}

inline void
Console_Step(console *Console)
{
    CPU_Step(Console->CPU);
    PPU_Step(Console->PPU);
    PPU_Step(Console->PPU);
    PPU_Step(Console->PPU);
    APU_Step(Console->APU);
}

static void
Console_Reset(console *Console)
{
    Gamepad_Reset(Console->Gamepad);
    Cart_Reset(Console->Cart);
    APU_Reset(Console->APU);
    PPU_Reset(Console->PPU);
    CPU_Reset(Console->CPU);
}

static void
Console_Free(console *Console)
{
    Gamepad_Free(Console->Gamepad);
    Cart_Free(Console->Cart);
    APU_Free(Console->APU);
    PPU_Free(Console->PPU);
    CPU_Free(Console->CPU);
    Api_Free(Console);
}

static console*
Console_Create(cart *Cart)
{
    console *Console = (console *)Api_Malloc(sizeof(console));
    Console->Cart = Cart;
    Console->Cart->Console = Console;
    Console->Gamepad = Gamepad_Create();
    Console->APU = APU_Create(Console);
    Console->PPU = PPU_Create(Console);
    Console->CPU = CPU_Create(Console);
    return Console;
}