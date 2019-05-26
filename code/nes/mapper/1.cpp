
static u8
Mapper1_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[Address >> 12][Address & 0xFFF]; break;
        case 1: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF]; break;
        case 2: break;
        case 3: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case 4: case 5: return Cart->Rom[0][Address & 0x3FFF];
        case 6: case 7: return Cart->Rom[1][Address & 0x3FFF];
    }

    return 0; // error
}

static void
Mapper1_Update(cart *Cart, u16 Address)
{
    Cart->Mapper1.SR >>= 1;
    Cart->Mapper1.SR &= 0x1F;
    u8 Mode = (Address & 0x7FFF) >> 13;
    switch (Mode)
    {
        case 0:
        {
            Cart->Mapper1.CTRL = Cart->Mapper1.SR;
            Cart_SetMirroring(Cart, Cart->Mapper1.CTRL & 3);
        } break;
        case 1:
        {
            if (Cart->Mapper1.CTRL & 0x10)
                Cart->Chr[0] = Cart->ChrRom + ((Cart->Mapper1.SR % Cart->ChrRomCount) << 12);
            else
            {
                u8 Bank = (Cart->Mapper1.SR & 0xFE);
                Cart->Chr[0] = Cart->ChrRom + ((Bank % Cart->ChrRomCount) << 12);
                Cart->Chr[1] = Cart->Chr[0] + 0x1000;
            }
        } break;
        case 2: 
        {
            if (Cart->Mapper1.CTRL & 0x10)
                Cart->Chr[1] = Cart->ChrRom + ((Cart->Mapper1.SR % Cart->ChrRomCount) << 12);
        } break;

        case 3: 
        {
            Cart->Rom[0] = Cart->PrgRom + ((Cart->Mapper1.SR % Cart->PrgRomCount) << 14);
        } break;
    }
    Cart->Mapper1.SR = 0x20;
}

static void
Mapper1_Register(cart *Cart, u16 Address, u8 Value)
{
    if (Value & 0x80)
    {
        Cart->Mapper1.SR = 0x20;
        // Cart->Mapper1.CTRL = 0x0C;
    }
    else
    {
        Cart->Mapper1.SR >>= 1;
        Cart->Mapper1.SR |= (Value & 1) << 5;
        if (Cart->Mapper1.SR & 0x1)
            Mapper1_Update(Cart, Address);
    }
}

static void
Mapper1_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[Address >> 12][Address & 0xFFF] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        default: Mapper1_Register(Cart, Address, Value); break;
    }
}

static void
Mapper1_Init(cart *Cart)
{
    Cart->ChrRomCount <<= 1;
            Cart->Read = Mapper1_Read;
            Cart->Write = Mapper1_Write;
            Cart->Chr[0] = Cart->ChrRom;
            Cart->Chr[1] = Cart->ChrRom + 0x1000;
            Cart->Ram = Cart->PrgRam;
            Cart->Rom[0] = Cart->PrgRom;
            Cart->Rom[1] = Cart->PrgRom + (0x4000 * (Cart->PrgRomCount - 1));
}