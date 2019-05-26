
static u8
Mapper0_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[0][Address]; break;
        case 1: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF]; break;
        case 2: break;
        case 3: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case 4: case 5: return Cart->Rom[0][Address & 0x3FFF];
        case 6: case 7: return Cart->Rom[1][Address & 0x3FFF];
    }

    return 0; // error
}

static void
Mapper0_Write(cart *Cart, u16 Address, u8 Value)
{
    if (Address < 0x4000)
        Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value;
    else if (0x6000 <= Address && Address < 0x8000 && Cart->Ram)
        Cart->Ram[(Address & 0x1FFF)] = Value;
}

static void
Mapper0_Init(cart *Cart)
{
    Cart->Read = Mapper0_Read;
    Cart->Write = Mapper0_Write;
    Cart->Chr[0] = Cart->ChrRom;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom;
    Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
}