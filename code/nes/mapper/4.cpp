
static void
Mapper4_IRQ(cart *Cart, u8 IRQNext)
{
    if (IRQNext && !Cart->Mapper4.IRQLast)
    {
        if (Cart->Mapper4.IRQReload)
        {
            Cart->Mapper4.IRQCounter = Cart->Mapper4.IRQDivider;
            Cart->Mapper4.IRQReload = 0x00;
        }
        else if (Cart->Mapper4.IRQCounter == 1)
        {
            --Cart->Mapper4.IRQCounter;
            if (Cart->Mapper4.IRQEnabled)
                Console_SetIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
        }
        else if (Cart->Mapper4.IRQCounter == 0)
        {
            Cart->Mapper4.IRQCounter = Cart->Mapper4.IRQDivider;
        }
        else --Cart->Mapper4.IRQCounter;
    }
    Cart->Mapper4.IRQLast = IRQNext;
}

static u8
Mapper4_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0:
        {
            u16 Chr = Address >> 10;
            Mapper4_IRQ(Cart, (Chr & 4));
            return Cart->Chr[Chr][Address & 0x3FF];
        } break;
        case 1: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF];
        case 2: break;
        case 3: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case 4: return Cart->Rom[0][Address & 0x1FFF];
        case 5: return Cart->Rom[1][Address & 0x1FFF];
        case 6: return Cart->Rom[2][Address & 0x1FFF];
        case 7: return Cart->Rom[3][Address & 0x1FFF];
    }

    return 0; // error
}

static void
Mapper4_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[Address >> 10][Address & 0x3FF] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        case 4:
        {
            if (Address & 1)
            {
                u8 Bank = Cart->Mapper4.BankSelect;
                switch (Bank & 0x7)
                {
                    case 0:
                    case 1:
                    {
                        Value %= Cart->ChrRomCount;
                        u8 Offset = (Bank & 0x80) ? 4 : 0;
                        Offset |= ((Bank & 0x7) << 1);
                        Cart->Chr[Offset | 0] = Cart->ChrRom + ((Value & 0xFE) << 10);
                        Cart->Chr[Offset | 1] = Cart->ChrRom + ((Value | 1) << 10);
                    } break;
                    
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    {
                        Value %= Cart->ChrRomCount;
                        u8 Offset = (Bank & 0x80) ? 0 : 4;
                        Cart->Chr[Offset | ((Bank & 0x7) - 2)] = Cart->ChrRom + (Value << 10);
                    } break;
                    
                    case 6:
                    {
                        Value &= 0x3F;
                        u8 Offset = (Bank & 0x40) ? 2 : 0;
                        Cart->Rom[0 ^ Offset] = Cart->PrgRom + ((Value % Cart->PrgRomCount) << 13);
                        Cart->Rom[2 ^ Offset] = Cart->PrgRom + ((Cart->PrgRomCount - 2) << 13);
                    } break;
                    case 7:
                    {
                        Value &= 0x3F;
                        Cart->Rom[1] = Cart->PrgRom + ((Value % Cart->PrgRomCount) << 13);
                    } break;
                }
            }
            else
                Cart->Mapper4.BankSelect = Value;
        } break;
        case 5:
        {
            if (Address & 1)
                Cart->Mapper4.PrgRamProtect = Value;
            else
                Cart_SetMirroring(Cart, (Value & 1) | 2);
        } break;
        case 6:
        {
            if (Address & 1)
                Cart->Mapper4.IRQReload = 0xFF;
            else
                Cart->Mapper4.IRQDivider = Value;
        } break;
        case 7:
        {
            if (Address & 1)
                Cart->Mapper4.IRQEnabled = 0xFF;
            else
            {
                Cart->Mapper4.IRQEnabled = 0x00;
                Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
            }
        } break;
    }
}

static void
Mapper4_Init(cart *Cart)
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
}
