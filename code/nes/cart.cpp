struct cart;

typedef u8 cart_read_func (cart *Cart, u16 Address);
typedef void cart_write_func (cart *Cart, u16 Address, u8 Value);
typedef void cart_step_func (cart *Cart);

struct cart
{
    console *Console;
    cart_read_func *Read;
    cart_write_func *Write;
    cart_step_func *Step;
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
    
    u8 PrgRomCount;
    u8 ChrRomCount;
    u16 PrgRamSize;
    u16 Mapper;

    union 
    {
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
        } Mapper5;

        struct
        {
            u8 Mode;
            u8 IRQLatch;
            u8 IRQCounter;
            u8 IRQCtrl;
            u16 IRQPrescaler;
        } Mapper24;
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
    if (Cart->Step) Cart->Step(Cart);
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
Cart_SetMirroring(cart *Cart, u8 Mirroring)
{
    switch (Mirroring)
    {
        case 0:
        {
            Cart->Nametable[0] = Cart->NametableLo;
            Cart->Nametable[1] = Cart->NametableLo;
            Cart->Nametable[2] = Cart->NametableLo;
            Cart->Nametable[3] = Cart->NametableLo;
        } break;
        case 1:
        {
            Cart->Nametable[0] = Cart->NametableHi;
            Cart->Nametable[1] = Cart->NametableHi;
            Cart->Nametable[2] = Cart->NametableHi;
            Cart->Nametable[3] = Cart->NametableHi;
        } break;
        case 2:
        {
            Cart->Nametable[0] = Cart->NametableLo;
            Cart->Nametable[1] = Cart->NametableHi;
            Cart->Nametable[2] = Cart->NametableLo;
            Cart->Nametable[3] = Cart->NametableHi;
        } break;
        case 3:
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
        case 0: 
        {
            Cart->Read = Mapper0_Read;
            Cart->Write = Mapper0_Write;
            Cart->Chr[0] = Cart->ChrRom;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom;
            Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
        } break;

        case 1:
        {
            Cart->ChrRomCount <<= 1;
            Cart->Read = Mapper1_Read;
            Cart->Write = Mapper1_Write;
            Cart->Chr[0] = Cart->ChrRom;
            Cart->Chr[1] = Cart->ChrRom + 0x1000;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom;
            Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
        } break;

        case 2:
        {
            Cart->Read = Mapper0_Read;
            Cart->Write = Mapper2_Write;
            Cart->Chr[0] = Cart->ChrRom;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom;
            Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
        } break;

        case 3:
        {
            Cart->Read = Mapper0_Read;
            Cart->Write = Mapper3_Write;
            Cart->Chr[0] = Cart->ChrRom;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom;
            Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
        } break;

        case 4:
        {
            Cart->PrgRomCount <<= 1;
            Cart->ChrRomCount <<= 3;
            Cart->Read = Mapper4_Read;
            Cart->Write = Mapper4_Write;
            for (u8 i = 0; i < 8; ++i)
                Cart->Chr[i] = Cart->ChrRom + i * 0x400;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
            Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
            Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
            Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
        } break;

        case 7:
        {
            Cart->Read = Mapper7_Read;
            Cart->Write = Mapper7_Write;
            Cart->Chr[0] = Cart->ChrRom;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom;
        } break;

        case 24:
        {
            Cart->PrgRomCount <<= 1;
            Cart->ChrRomCount <<= 3;
            Cart->Read = Mapper24_Read;
            Cart->Write = Mapper24_Write;
            Cart->Step = Mapper24_Step;
            for (u8 i = 0; i < 8; ++i)
                Cart->Chr[i] = Cart->ChrRom + i * 0x400;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
            Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
            Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
            Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
        } break;

        default: 
        {
            Cart_Free(Cart);
            Cart = 0;
        } break; // unsupported mapper
    }

    return Cart;
}
