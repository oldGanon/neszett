
static u8
Mapper7_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[0][Address];
        case 1: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF];
        case 2: break;
        case 3: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case 4: case 5: case 6: case 7: return Cart->Rom[0][Address & 0x7FFF];
    }

    return 0; // error
}

static void
Mapper7_Write(cart *Cart, u16 Address, u8 Value)
{
    if (Address < 0x2000)
        Cart->Chr[0][Address] = Value;
    else if (Address < 0x4000)
        Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value;
    else if(0x6000 <= Address && Address < 0x8000 && Cart->Ram)
        Cart->Ram[(Address & 0x1FFF)] = Value;
    else
    {
        u32 PrgRom = (Value & 7);
        Cart->Rom[0] = Cart->PrgRom + (0x8000 * PrgRom);
        Cart_SetMirroring(Cart, (Value >> 4) & 1);
    }
}

static void
Mapper7_Init(cart *Cart)
{
    Cart->Read = Mapper7_Read;
    Cart->Write = Mapper7_Write;
    Cart->Chr[0] = Cart->ChrRom;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom;
}
