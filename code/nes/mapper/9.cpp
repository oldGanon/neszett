
static u8
Mapper9_Read(cart *Cart, u16 Address)
{
    switch (Address >> 12)
    {
        case 0:
        {
            u8 Value = Cart->Chr[Cart->Mapper9.ChrLatch[0]][Address & 0xFFF];
            if (Address == 0xFD8) Cart->Mapper9.ChrLatch[0] = 0;
            if (Address == 0xFE8) Cart->Mapper9.ChrLatch[0] = 1;
            return Value;
        } break;

        case 1:
        {
            u8 Value = Cart->Chr[Cart->Mapper9.ChrLatch[1]][Address & 0xFFF];
            if (0x1FD8 <= Address && Address <= 0x1FDF) Cart->Mapper9.ChrLatch[1] = 2;
            if (0x1FE8 <= Address && Address <= 0x1FEF) Cart->Mapper9.ChrLatch[1] = 3;
            return Value;
        } break;

        case  2: case  3: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF];
        case  4: case  5: break;
        case  6: case  7: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case  8: case  9: return Cart->Rom[0][Address & 0x1FFF];
        case 10: case 11: return Cart->Rom[1][Address & 0x1FFF];
        case 12: case 13: return Cart->Rom[2][Address & 0x1FFF];
        case 14: case 15: return Cart->Rom[3][Address & 0x1FFF];
    }

    return 0; // error
}

static void
Mapper9_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 12)
    {
        case 0: Cart->Chr[Cart->Mapper9.ChrLatch[0]][Address & 0xFFF] = Value; break;
        case 1: Cart->Chr[Cart->Mapper9.ChrLatch[1]][Address & 0xFFF] = Value; break;
        case 2: case 3: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 6: case 7: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        case 0xA: Cart->Rom[0] = Cart->PrgRom + (0x2000 * ((Value & 0xF) % Cart->PrgRomCount)); break;
        case 0xB: Cart->Chr[0] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xC: Cart->Chr[1] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xD: Cart->Chr[2] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xE: Cart->Chr[3] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xF: Cart_SetMirroring(Cart, (Value & 1) | 2); break;
        default: break;
    }
}

static void
Mapper9_Init(cart *Cart)
{
    Cart->PrgRomCount <<= 1;
    Cart->ChrRomCount <<= 1;
    Cart->Read = Mapper9_Read;
    Cart->Write = Mapper9_Write;
    for (u8 i = 0; i < 4; ++i)
        Cart->Chr[i] = Cart->ChrRom + i * 0x1000;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 3));
    Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 3));
    Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
    Cart->Mapper9.ChrLatch[0] = 0;
    Cart->Mapper9.ChrLatch[1] = 2;
}
