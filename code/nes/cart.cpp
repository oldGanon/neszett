struct cart;

typedef u8 cart_read_func (cart *Cart, u16 Address);
typedef void cart_write_func (cart *Cart, u16 Address, u8 Value);
typedef void cart_step_func (cart *Cart);
typedef f32 cart_audio_func (cart *Cart);

struct vrc_irq
{
    u8 IRQLatch;
    u8 IRQCounter;
    u8 IRQCtrl;
    u16 IRQPrescaler;
};

struct cart
{
    console *Console;
    cart_read_func *Read;
    cart_write_func *Write;
    cart_step_func *Step;
    cart_audio_func *Audio;
    file Save;

    u8 *ChrRom;
    u8 *PrgRam;
    u8 *PrgRom;
    u8 *NametableLo;
    u8 *NametableHi;

    u8 *Chr[8];
    u8 *Ram;
    u8 *Rom[4];
    u8 *Nametable[4];
    
    u16 PrgRomCount;
    u16 ChrRomCount;
    u16 PrgRamCount;
    u16 Mapper;

    union 
    {
        vrc_irq VrcIrq;

        struct
        {
            u8 SR;
            u8 CTRL;
        } Mapper1;

        struct
        {
            u8 BankSelect;
            u8 PrgRamProtect;
            u8 IRQLast;
            u8 IRQDivider;
            u8 IRQReload;
            u8 IRQCounter;
            u8 IRQEnabled;
        } Mapper4;

        struct
        {
            u8 PrgMode;
            u8 ChrMode;
            u8 RamProtect1;
            u8 RamProtect2;
            u8 RamExtend;
            u8 NametableMap;
            u8 FillModeTile;
            u8 FillModeColor;
            u8 PrgBanks[4];
            u16 ChrBanks[8];
            u16 ChrBGBanks[4];
            u8 ChrBankHi;
            u8 VSplitMode;
            u8 VSplitScroll;
            u8 VSplitBank;
            u8 IRQScanline;
            u8 IRQEnable;
            u8 IRQPending;
            u16 IRQLastAddr;
            u16 IRQAddrCount;
            u8 PPUReading;
            u8 PPUIdleCount;
            u8 Scanline;
            u8 InFrame;
            u8 Mul[2];
        } Mapper5;

        struct
        {
            u8 ChrLatch[2];
        } Mapper9;

        struct
        {
            vrc_irq VrcIrq;
            u8 PrgSwapMode;
            u16 Chr[8];
        } Mapper23;

        struct
        {
            vrc_irq VrcIrq;
            u8 Mode;

            struct
            {
                u16 Counter;
                u16 Period;
                u8 Duty;
                u8 DutyCycle;
                u8 Volume;
                u8 Mode;
                u8 Enable;
            } Square[2];
            u8 SquareHalt;
            u8 SquareFreq;

            u16 SawCounter;
            u16 SawPeriod;
            u8 SawStep;
            u8 SawAccumRate;
            u8 SawAccum;
            u8 SawEnable;
        } Mapper24;

        struct
        {
            u8 Command;
            u8 IRQCtrl;
            u16 IRQCounter;

            u16 SquareCounter[3];
            u16 SquarePeriod[3];
            u8 SquareVolume[3];
            u8 ChannelEnable;
            u8 AudioRegister;
        } Mapper69;
    };
};

static void Cart_SetMirroring(cart *Cart, u8 Mirroring);

#include "mapper.cpp"

inline u8
Cart_Read(cart *Cart, u16 Address)
{
    return Cart->Read(Cart, Address);
}

inline void
Cart_Write(cart *Cart, u16 Address, u8 Value)
{
    Cart->Write(Cart, Address, Value);
}

inline void
Cart_Step(cart *Cart)
{
    if (Cart->Step)
        Cart->Step(Cart);
}

inline f32
Cart_Audio(cart *Cart)
{
    if (Cart->Audio)
        return Cart->Audio(Cart);
    return 0;
}

static void
Cart_Free(cart *Cart)
{
    if (File_Valid(Cart->Save))
        File_WriteData(Cart->Save, 0, Cart->PrgRam, 0x2000);

    Api_Free(Cart->PrgRom);
    Api_Free(Cart->ChrRom);
    Api_Free(Cart->PrgRam);

    switch (Cart->Mapper)
    {
        case 0:
        {
            Api_Free(Cart->NametableLo);
            Api_Free(Cart->NametableHi);
        } break;
    }

    Api_Free(Cart);
}

static void
Cart_Reset(cart *Cart)
{
    Memory_Set(Cart->Nametable[0], 0, 0x400);
    Memory_Set(Cart->Nametable[3], 0, 0x400);
}

static void
Cart_Power(cart *Cart)
{
    Memory_Set(Cart->Nametable[0], 0, 0x400);
    Memory_Set(Cart->Nametable[3], 0, 0x400);
}

static void
Cart_SetMirroring(cart *Cart, u8 Mirroring)
{
    switch (Mirroring)
    {
        case 0: // Screen A
        {
            Cart->Nametable[0] = Cart->NametableLo;
            Cart->Nametable[1] = Cart->NametableLo;
            Cart->Nametable[2] = Cart->NametableLo;
            Cart->Nametable[3] = Cart->NametableLo;
        } break;
        case 1: // Screen B
        {
            Cart->Nametable[0] = Cart->NametableHi;
            Cart->Nametable[1] = Cart->NametableHi;
            Cart->Nametable[2] = Cart->NametableHi;
            Cart->Nametable[3] = Cart->NametableHi;
        } break;
        case 2: // Mirror Vertical
        {
            Cart->Nametable[0] = Cart->NametableLo;
            Cart->Nametable[1] = Cart->NametableHi;
            Cart->Nametable[2] = Cart->NametableLo;
            Cart->Nametable[3] = Cart->NametableHi;
        } break;
        case 3: // Mirror Horizontal
        {
            Cart->Nametable[0] = Cart->NametableLo;
            Cart->Nametable[1] = Cart->NametableLo;
            Cart->Nametable[2] = Cart->NametableHi;
            Cart->Nametable[3] = Cart->NametableHi;
        } break;
    }
}

static cart*
Cart_Init(cart *Cart, u8 Mirroring)
{
    Cart->NametableLo = (u8 *)Api_Malloc(0x400);
    Cart->NametableHi = (u8 *)Api_Malloc(0x400);

    Cart_SetMirroring(Cart, (Mirroring) ? 2 : 3);

    switch (Cart->Mapper)
    {
        case 0: Mapper0_Init(Cart); break;
        case 1: Mapper1_Init(Cart); break;
        case 2: Mapper2_Init(Cart); break;
        case 3: Mapper3_Init(Cart); break;
        case 4: Mapper4_Init(Cart); break;
        case 5: Mapper5_Init(Cart); break;
        case 7: Mapper7_Init(Cart); break;
        case 9: Mapper9_Init(Cart); break;
        case 10: Mapper10_Init(Cart); break;
        case 23: Mapper23_Init(Cart); break;
        case 24: Mapper24_Init(Cart); break;
        case 69: Mapper69_Init(Cart); break;

        default: 
        {
            Cart_Free(Cart);
            Cart = 0;
        } break; // unsupported mapper
    }

    return Cart;
}
