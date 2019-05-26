
static void
Mapper3_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[0][Address] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        default: Cart->Chr[0] = Cart->ChrRom + ((Value % Cart->ChrRomCount) << 13); break;
    }   
}

static void
Mapper3_Init(cart *Cart)
{
	Cart->Read = Mapper0_Read;
    Cart->Write = Mapper3_Write;
    Cart->Chr[0] = Cart->ChrRom;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom;
    Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
}
