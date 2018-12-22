//
// MAPPER 0
//

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
    else if(0x6000 <= Address && Address < 0x8000 && Cart->Ram)
        Cart->Ram[(Address & 0x1FFF)] = Value;
}

//
// MAPPER 1
//

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
                u8 Bank = (Cart->Mapper1.SR >> 1);
                Cart->Chr[0] = Cart->ChrRom + ((Bank % Cart->ChrRomCount) << 13);
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
        Cart->Mapper1.CTRL = 0x0C;
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

//
// MAPPER 2
//

static void
Mapper2_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[0][Address] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        default: Cart->Rom[0] = Cart->PrgRom + ((Value % Cart->PrgRomCount) << 14); break;
    }   
}

//
// MAPPER 3
//

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

//
// Mapper 4
//

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
        case 1: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF]; break;
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

//
// Mapper 5
//

static void
Mapper5_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address)
    {
        case 0x5100: Cart->Mapper5.PrgMode = Value & 0x3; break;
        case 0x5101: Cart->Mapper5.ChrMode = Value & 0x3; break;
        case 0x5102: Cart->Mapper5.RamProtect1 = Value & 0x3; break;
        case 0x5103: Cart->Mapper5.RamProtect2 = Value & 0x3; break;
        case 0x5104: Cart->Mapper5.RamExtend = Value & 0x3; break;
        case 0x5105: Cart->Mapper5.NametableMap = Value; break;
        case 0x5106: Cart->Mapper5.FillModeTile = Value; break;
        case 0x5107: Cart->Mapper5.FillModeColor = Value; break;

        case 5113: Cart->Ram = Cart->PrgRam + ((Value & 0x7) << 14); break;

        case 0x5114:
        {
            if (Cart->Mapper5.PrgMode == 3)
            {
                mi Offset = (mi)Value;
                Offset &= (Value & 0x80) ? 0x7 : 0x7F;
                Offset <<= 13;
                u8 *Src = (Value & 0x80) ? Cart->PrgRam : Cart->PrgRom;
                Cart->Rom[0] = Src + Offset;
            }
        } break;
        case 0x5115:
        {
            u8 *Src;
            mi Offset;
            if (Value & 0x80)
            {
                Src = Cart->PrgRom;
                Offset = ((mi)Value & 0x7F) << 13;
            }
            else
            {
                Src = Cart->PrgRam;
                Offset = ((mi)Value & 0x7) << 13;
            }
            switch (Cart->Mapper5.PrgMode)
            {
                default: break;
                case 1:
                case 2:
                {
                    Cart->Rom[0] = Src + (Offset & 0xFC000);
                    Cart->Rom[1] = Src + (Offset | 0x02000);
                }
                case 3: Cart->Rom[1] = Src + Offset; break;
            }
        } break;
        case 0x5116:
        {
            u8 *Src;
            if (Value & 0x80)
                Src = Cart->PrgRom + (((mi)Value & 0x7F) << 13);
            else
                Src = Cart->PrgRam + (((mi)Value & 0x7) << 13);

            if (Cart->Mapper5.PrgMode & 0x2)
                Cart->Rom[2] = Src;
        } break;
        case 0x5117:
        {
            switch (Cart->Mapper5.PrgMode)
            {
                case 0:
                {
                    mi Offset = (((mi)Value & 0x7C) << 13);
                    Cart->Rom[0] = Cart->PrgRom + Offset;
                    Cart->Rom[1] = Cart->PrgRom + (Offset | 0x02000);
                    Cart->Rom[2] = Cart->PrgRom + (Offset | 0x04000);
                    Cart->Rom[3] = Cart->PrgRom + (Offset | 0x06000);
                } break;
                case 1:
                {
                    mi Offset = ((mi)Value & 0x7E) << 13;
                    Cart->Rom[2] = Cart->PrgRom + Offset;
                    Cart->Rom[3] = Cart->PrgRom + (Offset | 0x02000);
                } break;
                case 2:
                case 3: Cart->Rom[3] = Cart->PrgRom + (((mi)Value & 0x7F) << 13); break;
            }
        } break;
        
        default: break;
    }
}

//
// MAPPER 7
//

static u8
Mapper7_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[0][Address]; break;
        case 1: return Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF]; break;
        case 2: break;
        case 3: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case 4: case 5: case 6: case 7: return Cart->Rom[0][Address & 0x7FFF];
    }

    return 0; // error
}

static void
Mapper7_Write(cart *Cart, u16 Address, u8 Value)
{
    if (Address < 0x4000)
        Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value;
    else if(0x6000 <= Address && Address < 0x8000 && Cart->Ram)
        Cart->Ram[(Address & 0x1FFF)] = Value;
    else
    {
        u32 PrgRom = (Value & 7);
        Cart->Rom[0] = Cart->PrgRom + (0x4000 * PrgRom);
    }
}
