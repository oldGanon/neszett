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
    else if (0x6000 <= Address && Address < 0x8000 && Cart->Ram)
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

//
// Mapper 5
//

static void
Mapper5_UpdatePrgMapping(cart *Cart)
{
    switch(Cart->Mapper5.PrgMode)
    {
        case 0:
        {
            Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->Mapper5.PrgBanks[3] & 0x7C));
            Cart->Rom[1] = Cart->Rom[0] + 0x2000;
            Cart->Rom[2] = Cart->Rom[1] + 0x2000;
            Cart->Rom[3] = Cart->Rom[2] + 0x2000;
        } break;

        case 1:
        {
            u8 *Src = (Cart->Mapper5.PrgBanks[1] & 0x80) ? Cart->PrgRom : Cart->PrgRam;
            Cart->Rom[0] = Src + (0x2000 * (Cart->Mapper5.PrgBanks[1] & 0x7E));
            Cart->Rom[1] = Cart->Rom[0] + 0x2000;
            Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->Mapper5.PrgBanks[3] & 0x7E));
            Cart->Rom[3] = Cart->Rom[2] + 0x2000;
        } break;
        
        case 2:
        {
            u8 *Src = (Cart->Mapper5.PrgBanks[1] & 0x80) ? Cart->PrgRom : Cart->PrgRam;
            Cart->Rom[0] = Src + (0x2000 * (Cart->Mapper5.PrgBanks[1] & 0x7E));
            Cart->Rom[1] = Cart->Rom[0] + 0x2000;
            Src = (Cart->Mapper5.PrgBanks[2] & 0x80) ? Cart->PrgRom : Cart->PrgRam;
            Cart->Rom[2] = Src + (0x2000 * (Cart->Mapper5.PrgBanks[2] & 0x7F));
            Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->Mapper5.PrgBanks[3] & 0x7F));
        } break;
        
        case 3:
        {
            u8 *Src = (Cart->Mapper5.PrgBanks[0] & 0x80) ? Cart->PrgRom : Cart->PrgRam;
            Cart->Rom[0] = Src + (0x2000 * (Cart->Mapper5.PrgBanks[0] & 0x7F));
            Src = (Cart->Mapper5.PrgBanks[1] & 0x80) ? Cart->PrgRom : Cart->PrgRam;
            Cart->Rom[1] = Src + (0x2000 * (Cart->Mapper5.PrgBanks[1] & 0x7F));
            Src = (Cart->Mapper5.PrgBanks[2] & 0x80) ? Cart->PrgRom : Cart->PrgRam;
            Cart->Rom[2] = Src + (0x2000 * (Cart->Mapper5.PrgBanks[2] & 0x7F));
            Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->Mapper5.PrgBanks[3] & 0x7F));
        } break;
        
        default: break;
    }
}

static void
Mapper5_UpdateChrMapping(cart *Cart)
{
    switch (Cart->Mapper5.ChrMode)
    {
        case 0:
        {
            Cart->Chr[0] = Cart->ChrRom + 0x2000 * (Cart->Mapper5.ChrBanks[7] % Cart->ChrRomCount);
            Cart->Chr[1] = Cart->Chr[0] + 0x0400;
            Cart->Chr[2] = Cart->Chr[0] + 0x0800;
            Cart->Chr[3] = Cart->Chr[0] + 0x0C00;
            Cart->Chr[4] = Cart->Chr[0] + 0x1000;
            Cart->Chr[5] = Cart->Chr[0] + 0x1400;
            Cart->Chr[6] = Cart->Chr[0] + 0x1800;
            Cart->Chr[7] = Cart->Chr[0] + 0x1C00;
        } break;

        case 1:
        {
            u16 ChrBankCount = Cart->ChrRomCount << 1;
            Cart->Chr[0] = Cart->ChrRom + 0x1000 * (Cart->Mapper5.ChrBanks[3] % ChrBankCount);
            Cart->Chr[1] = Cart->Chr[0] + 0x400;
            Cart->Chr[2] = Cart->Chr[0] + 0x800;
            Cart->Chr[3] = Cart->Chr[0] + 0xC00;
            Cart->Chr[4] = Cart->ChrRom + 0x1000 * (Cart->Mapper5.ChrBanks[7] % ChrBankCount);
            Cart->Chr[5] = Cart->Chr[4] + 0x400;
            Cart->Chr[6] = Cart->Chr[4] + 0x800;
            Cart->Chr[7] = Cart->Chr[4] + 0xC00;
        } break;

        case 2:
        {
            u16 ChrBankCount = Cart->ChrRomCount << 2;
            Cart->Chr[0] = Cart->ChrRom + 0x800 * (Cart->Mapper5.ChrBanks[1] % ChrBankCount);
            Cart->Chr[1] = Cart->Chr[0] + 0x400;
            Cart->Chr[2] = Cart->ChrRom + 0x800 * (Cart->Mapper5.ChrBanks[3] % ChrBankCount);
            Cart->Chr[3] = Cart->Chr[2] + 0x400;
            Cart->Chr[4] = Cart->ChrRom + 0x800 * (Cart->Mapper5.ChrBanks[5] % ChrBankCount);
            Cart->Chr[5] = Cart->Chr[4] + 0x400;
            Cart->Chr[6] = Cart->ChrRom + 0x800 * (Cart->Mapper5.ChrBanks[7] % ChrBankCount);
            Cart->Chr[7] = Cart->Chr[6] + 0x400;
        } break;

        case 3:
        {
            u16 ChrBankCount = Cart->ChrRomCount << 3;
            Cart->Chr[0] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[0] % ChrBankCount);
            Cart->Chr[1] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[1] % ChrBankCount);
            Cart->Chr[2] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[2] % ChrBankCount);
            Cart->Chr[3] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[3] % ChrBankCount);
            Cart->Chr[4] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[4] % ChrBankCount);
            Cart->Chr[5] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[5] % ChrBankCount);
            Cart->Chr[6] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[6] % ChrBankCount);
            Cart->Chr[7] = Cart->ChrRom + 0x400 * (Cart->Mapper5.ChrBanks[7] % ChrBankCount);
        } break;
    }
}

static void
Mapper5_UpdateChrBGMapping(cart *Cart)
{
    switch (Cart->Mapper5.ChrMode)
    {
        case 0:
        { } break;

        case 1:
        { } break;

        case 2:
        { } break;

        case 3:
        { } break;
    }
}

static void
Mapper5_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address)
    {
        case 0x5100: Cart->Mapper5.PrgMode = Value & 0x3; 
                     Mapper5_UpdatePrgMapping(Cart);
                     break;
        case 0x5101: Cart->Mapper5.ChrMode = Value & 0x3;
                     Mapper5_UpdateChrMapping(Cart);
                     break;
        case 0x5102: Cart->Mapper5.RamProtect1 = Value & 0x3; break;
        case 0x5103: Cart->Mapper5.RamProtect2 = Value & 0x3; break;
        case 0x5104: Cart->Mapper5.RamExtend = Value & 0x3; break;
        case 0x5105: Cart->Mapper5.NametableMap = Value; break;
        case 0x5106: Cart->Mapper5.FillModeTile = Value; break;
        case 0x5107: Cart->Mapper5.FillModeColor = Value & 0x3; break;

        case 0x5113:
        {
            Cart->Ram = Cart->PrgRam + (0x2000 * (Value & 7));
        } break;
        case 0x5114:
        case 0x5115:
        case 0x5116:
        case 0x5117:
        {
            Cart->Mapper5.PrgBanks[Address-0x5114] = Value;
            Mapper5_UpdatePrgMapping(Cart);
        } break;
        
        case 0x5120:
        case 0x5121:
        case 0x5122:
        case 0x5123:
        case 0x5124:
        case 0x5125:
        case 0x5126:
        case 0x5127:
        {
            Cart->Mapper5.ChrBanks[Address-0x5120] = (Cart->Mapper5.ChrBankHi << 8) | Value;
            Mapper5_UpdateChrMapping(Cart);
        } break;

        case 0x5128:
        case 0x5129:
        case 0x512A:
        case 0x512B:
        {
            Cart->Mapper5.ChrBGBanks[Address-0x5128] = (Cart->Mapper5.ChrBankHi << 8) | Value;
            Mapper5_UpdateChrBGMapping(Cart);
        } break;

        case 0x5130: Cart->Mapper5.ChrBankHi = Value & 3; break;

        case 0x5200: Cart->Mapper5.VSplitMode = Value; break;
        case 0x5201: Cart->Mapper5.VSplitScroll = Value; break;
        case 0x5202: Cart->Mapper5.VSplitBank = Value; break;
        case 0x5203: Cart->Mapper5.IRQScanline = Value; break;
        case 0x5204: Cart->Mapper5.IRQEnable = Value & 0x80; break;
        case 0x5205: Cart->Mapper5.Mul[0] = Value; break;
        case 0x5206: Cart->Mapper5.Mul[1] = Value; break;

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

//
// MAPPER 9
//

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

//
// MAPPER 10
//

static u8
Mapper10_Read(cart *Cart, u16 Address)
{
    switch (Address >> 12)
    {
        case 0:
        {
            u8 Value = Cart->Chr[Cart->Mapper9.ChrLatch[0]][Address & 0xFFF];
            if (0x0FD8 <= Address && Address <= 0x0FDF) Cart->Mapper9.ChrLatch[0] = 0;
            if (0x0FE8 <= Address && Address <= 0x0FEF) Cart->Mapper9.ChrLatch[0] = 1;
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
        case  8: case  9: case 10: case 11: return Cart->Rom[0][Address & 0x3FFF];
        case 12: case 13: case 14: case 15: return Cart->Rom[1][Address & 0x3FFF];
    }

    return 0; // error
}

static void
Mapper10_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 12)
    {
        case 0: Cart->Chr[Cart->Mapper9.ChrLatch[0]][Address & 0xFFF] = Value; break;
        case 1: Cart->Chr[Cart->Mapper9.ChrLatch[1]][Address & 0xFFF] = Value; break;
        case 2: case 3: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 6: case 7: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        case 0xA: Cart->Rom[0] = Cart->PrgRom + (0x4000 * ((Value & 0xF) % Cart->PrgRomCount)); break;
        case 0xB: Cart->Chr[0] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xC: Cart->Chr[1] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xD: Cart->Chr[2] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xE: Cart->Chr[3] = Cart->ChrRom + (0x1000 * ((Value & 0x1F) % Cart->ChrRomCount)); break;
        case 0xF: Cart_SetMirroring(Cart, (Value & 1) | 2); break;
        default: break;
    }
}

//
// VRC
//

static void
VrcIrq_Ctrl(cart *Cart, u8 Value)
{
    Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
    Cart->VrcIrq.IRQCtrl = Value & 7;
    if (Value & 2)
    {
        Cart->VrcIrq.IRQCounter = Cart->VrcIrq.IRQLatch;
        Cart->VrcIrq.IRQPrescaler = 341;
    }
}

static void
VrcIrq_Ack(cart *Cart)
{
    Cart->VrcIrq.IRQCtrl &= 5;
    Cart->VrcIrq.IRQCtrl |= (Cart->VrcIrq.IRQCtrl & 1) << 1;
    Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
}

static void
VrcIrq_Step(cart *Cart)
{
    if (!(Cart->VrcIrq.IRQCtrl & 2)) return;
    if (!(Cart->VrcIrq.IRQCtrl & 4))
    {
        Cart->VrcIrq.IRQPrescaler -= 3;
        if (Cart->VrcIrq.IRQPrescaler > 341)
            Cart->VrcIrq.IRQPrescaler = 341;
        else return;
    }

    if (Cart->VrcIrq.IRQCounter == 0xFF)
    {
        Cart->VrcIrq.IRQCounter = Cart->VrcIrq.IRQLatch;
        Console_SetIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
    }
    else ++Cart->VrcIrq.IRQCounter;
}

//
// MAPPER 23
//

static void
Mapper23_Step(cart *Cart)
{
    VrcIrq_Step(Cart);
}

static u8
Mapper23_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[Address >> 10][Address & 0x3FF];
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
Mapper23_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[Address >> 10][Address & 0x3FF] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        default:
        {
            Address = (Address & 0xF000) | (((Address >> 2) | Address) & 3);
            switch(Address)
            {
                case 0x8000: case 0x8001: case 0x8002: case 0x8003:
                {
                    u8 *Rom = Cart->PrgRom + (0x2000 * ((0x1F & Value) % Cart->PrgRomCount));
                    Cart->Rom[Cart->Mapper23.PrgSwapMode] = Rom;
                } break;

                case 0x9000: case 0x9001:
                {
                    Cart_SetMirroring(Cart, (Value ^ 2) & 3);
                } break;

                case 0x9002: case 0x9003:
                {
                    u8 NewMode = Value & 0x02;
                    if (Cart->Mapper23.PrgSwapMode != NewMode)
                    {
                        u8 *T = Cart->Rom[0];
                        Cart->Rom[0] = Cart->Rom[2];
                        Cart->Rom[2] = T;
                        Cart->Mapper23.PrgSwapMode = NewMode;
                    }
                } break;

                case 0xA000: case 0xA001: case 0xA002: case 0xA003:
                {
                    Cart->Rom[1] = Cart->PrgRom + (0x2000 * ((0x1F & Value) % Cart->PrgRomCount));
                } break;

                case 0xF000: Cart->VrcIrq.IRQLatch = (Cart->VrcIrq.IRQLatch & 0xF0) | (Value & 0xF); break;
                case 0xF001: Cart->VrcIrq.IRQLatch = (Cart->VrcIrq.IRQLatch & 0xF) | ((Value & 0xF) << 4); break;
                case 0xF002:
                {
                    VrcIrq_Ctrl(Cart, Value);
                } break;
                case 0xF003:
                {
                    VrcIrq_Ack(Cart);                    
                } break;

                default:
                {
                    u8 Chr = ((Address & 2) >> 1) | ((Address >> 11) - 0x16);
                    if (Address & 1)
                    {
                        Cart->Mapper23.Chr[Chr] = ((Cart->Mapper23.Chr[Chr] & 0xF) | ((Value & 0x1F) << 4)) % Cart->ChrRomCount;
                        Cart->Chr[Chr] = Cart->ChrRom + Cart->Mapper23.Chr[Chr] * 0x400;
                    }
                    else
                    {
                        Cart->Mapper23.Chr[Chr] = ((Cart->Mapper23.Chr[Chr] & 0x1F0) | (Value & 0xF)) % Cart->ChrRomCount;
                        Cart->Chr[Chr] = Cart->ChrRom + Cart->Mapper23.Chr[Chr] * 0x400;
                    }
                } break;
            }
        }
    }
}

//
// MAPPER 24
//

static void
Mapper24_Step(cart *Cart)
{
    // AUDIO
    if (!Cart->Mapper24.SquareHalt)
    for (u32 i = 0; i < 2; ++i)
    if (!(Cart->Mapper24.Square[i].Counter--))
    {
        Cart->Mapper24.Square[i].Counter = Cart->Mapper24.Square[i].Period >> Cart->Mapper24.SquareFreq;
        if (Cart->Mapper24.Square[i].Enable &&
            !(Cart->Mapper24.Square[i].Duty--))
            Cart->Mapper24.Square[i].Duty = 15;
    }

    if (!(Cart->Mapper24.SawCounter--))
    {
        Cart->Mapper24.SawCounter = Cart->Mapper24.SawPeriod;
        if (Cart->Mapper24.SawStep++ & 1)
            Cart->Mapper24.SawAccum += Cart->Mapper24.SawAccumRate;
        if (Cart->Mapper24.SawStep >= 14)
        {
            Cart->Mapper24.SawStep = 0;
            Cart->Mapper24.SawAccum = 0;
        }
    }

    // IRQ
    VrcIrq_Step(Cart);
}

static f32
Mapper24_Audio(cart *Cart)
{
    f32 Out = 0.0f;
    for (u32 i = 0; i < 2; ++i)
    {
        if (Cart->Mapper24.Square[i].Enable &&
            (Cart->Mapper24.Square[i].Mode ||
             Cart->Mapper24.Square[i].Duty <= Cart->Mapper24.Square[i].DutyCycle))
            Out -= Cart->Mapper24.Square[i].Volume;
    }

    if (Cart->Mapper24.SawEnable)
        Out -= Cart->Mapper24.SawAccum >> 3;

    return Out / 64.0f;
}

static u8
Mapper24_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[Address >> 10][Address & 0x3FF];
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
Mapper24_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[Address >> 10][Address & 0x3FF] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        default:
        {
            Address &= 0xF003;
            switch(Address)
            {
                case 0x8000: case 0x8001: case 0x8002: case 0x8003:
                {
                    u16 Bank = ((Value & 0xF) << 1) % Cart->PrgRomCount;
                    Cart->Rom[0] = Cart->PrgRom + (Bank * 0x2000);
                    Cart->Rom[1] = Cart->Rom[0] + 0x2000;
                } break;

                case 0x9000:
                {
                    Cart->Mapper24.Square[0].Volume = Value & 0xF;
                    Cart->Mapper24.Square[0].DutyCycle = (Value & 0x70) >> 4;
                    Cart->Mapper24.Square[0].Mode = Value & 0x80;
                } break;
                case 0x9001:
                {
                    Cart->Mapper24.Square[0].Period &= 0xFF00;
                    Cart->Mapper24.Square[0].Period |= Value;
                } break;
                case 0x9002:
                {
                    Cart->Mapper24.Square[0].Period &= 0xFF;
                    Cart->Mapper24.Square[0].Period |= (Value & 0xF) << 8;
                    if (Value & 0x80)
                        Cart->Mapper24.Square[0].Enable = 0xFF;
                    else
                    {
                        Cart->Mapper24.Square[0].Enable = 0x00;
                        Cart->Mapper24.Square[0].Duty = 15;
                    }
                } break;
                case 0x9003:
                {
                    Cart->Mapper24.SquareHalt = Value & 1;
                    if (Value & 4)
                        Cart->Mapper24.SquareFreq = 8;
                    else if (Value & 2)
                        Cart->Mapper24.SquareFreq = 4;
                    else
                        Cart->Mapper24.SquareFreq = 0;
                } break;

                case 0xA000:
                {
                    Cart->Mapper24.Square[1].Volume = Value & 0xF;
                    Cart->Mapper24.Square[1].DutyCycle = (Value & 0x70) >> 4;
                    Cart->Mapper24.Square[1].Mode = Value & 0x80;
                } break;
                case 0xA001:
                {
                    Cart->Mapper24.Square[1].Period &= 0xFF00;
                    Cart->Mapper24.Square[1].Period |= Value;
                } break;
                case 0xA002:
                {
                    Cart->Mapper24.Square[1].Period &= 0xFF;
                    Cart->Mapper24.Square[1].Period |= (Value & 0xF) << 8;
                    if (Value & 0x80)
                        Cart->Mapper24.Square[1].Enable = 0xFF;
                    else
                    {
                        Cart->Mapper24.Square[1].Enable = 0x00;
                        Cart->Mapper24.Square[1].Duty = 15;
                    }
                } break;

                case 0xB000: Cart->Mapper24.SawAccumRate = Value & 0x3F; break;
                case 0xB001:
                {
                    Cart->Mapper24.SawPeriod &= 0xFF00;
                    Cart->Mapper24.SawPeriod |= Value;
                } break;
                case 0xB002:
                {
                    Cart->Mapper24.SawPeriod &= 0xFF;
                    Cart->Mapper24.SawPeriod |= (Value & 0xF) << 8;
                    if (Value & 0x80)
                        Cart->Mapper24.SawEnable = 0xFF;
                    else
                    {
                        Cart->Mapper24.SawEnable = 0x00;
                        Cart->Mapper24.SawAccum = 0;
                    }

                } break;
                case 0xB003:
                {
                    Cart->Mapper24.Mode = Value;
                    Cart_SetMirroring(Cart, ((Cart->Mapper24.Mode >> 2) & 3) ^ 2);
                } break;
                
                case 0xC000: case 0xC001: case 0xC002: case 0xC003:
                {
                    u16 Bank = (Value & 0x1F) % Cart->PrgRomCount;
                    Cart->Rom[2] = Cart->PrgRom + (Bank * 0x2000);
                } break;

                case 0xD000: case 0xD001: case 0xD002: case 0xD003:
                {
                    u8 R = Address & 3;
                    u8 *Chr = Cart->ChrRom + (Value % Cart->ChrRomCount) * 0x400;
                    if ((Cart->Mapper24.Mode & 3) == 1)
                    {
                        R <<= 1;
                        Cart->Chr[R] = Chr;
                        Cart->Chr[R|1] = Chr + 0x400;
                    } 
                    else Cart->Chr[R] = Chr;
                } break;

                case 0xE000: case 0xE001: case 0xE002: case 0xE003:
                {
                    u8 R = (Address & 3);
                    u8 *Chr = Cart->ChrRom + (Value % Cart->ChrRomCount) * 0x400;
                    if (Cart->Mapper24.Mode & 2)
                    {
                        R <<= 1;
                        Cart->Chr[R|4] = Chr;
                        Cart->Chr[R|5] = Chr + 0x400;
                    }
                    else if (Cart->Mapper24.Mode & 1)
                    {
                    }
                    else Cart->Chr[R|4] = Chr;
                } break;

                case 0xF000: Cart->VrcIrq.IRQLatch = Value; break;
                case 0xF001:
                {
                    VrcIrq_Ctrl(Cart, Value);
                } break;
                case 0xF002:
                {
                    VrcIrq_Ack(Cart);
                } break;
            }
        }
    }
}

//
// MAPPER 69
//

static void
Mapper69_Step(cart *Cart)
{
    if (Cart->Mapper69.IRQCtrl & 0x80)
    {
        --Cart->Mapper69.IRQCounter;
        if (Cart->Mapper69.IRQCounter == 0xFFFF &&
            Cart->Mapper69.IRQCtrl & 0x1)
            Console_SetIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
    }
}

static f32
Mapper69_Audio(cart *Cart)
{
    f32 Out = 0.0f;
    return Out;
}

static u8
Mapper69_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[Address >> 10][Address & 0x3FF];
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
Mapper69_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[Address >> 10][Address & 0x3FF] = Value; break;
        case 1: Cart->Nametable[(Address >> 10) & 3][Address & 0x3FF] = Value; break;
        case 2: break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        case 4: Cart->Mapper69.Command = Value & 0xF; break;
        case 5:
        {
            if (Cart->Mapper69.Command <= 0x7)
                Cart->Chr[Cart->Mapper69.Command] = Cart->ChrRom + (Value % Cart->ChrRomCount) * 0x400;
            else if (Cart->Mapper69.Command == 0x8)
            {
                if (Value & 0x40)
                    Cart->Ram = Cart->PrgRam;
                else
                    Cart->Ram = Cart->PrgRom + ((Value & 0x1F) % Cart->PrgRomCount) * 0x2000;
            }
            else if (Cart->Mapper69.Command <= 0xB)
                Cart->Rom[Cart->Mapper69.Command - 0x9] = Cart->PrgRom + ((Value & 0x1F) % Cart->PrgRomCount) * 0x2000;
            else if (Cart->Mapper69.Command == 0xC)
                Cart_SetMirroring(Cart, (Value & 3) ^ 2);
            else if (Cart->Mapper69.Command == 0xD)
            {
                Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
                Cart->Mapper69.IRQCtrl = Value;
            }
            else if (Cart->Mapper69.Command == 0xE)
            {
                Cart->Mapper69.IRQCounter &= 0xFF00;
                Cart->Mapper69.IRQCounter |= Value;
            }
            else if (Cart->Mapper69.Command == 0xF)
            {
                Cart->Mapper69.IRQCounter &= 0xFF;
                Cart->Mapper69.IRQCounter |= Value << 8;
            }
        } break;
        case 6:
        {
            Cart->Mapper69.AudioRegister = Value & 0xF;
        } break;
        case 7:
        {
            switch (Cart->Mapper69.AudioRegister)
            {
                case 0x0:
                {
                    Cart->Mapper69.SquarePeriod[0] &= 0xFF00;
                    Cart->Mapper69.SquarePeriod[0] |= Value;
                } break;
                case 0x1:
                {
                    Cart->Mapper69.SquarePeriod[1] &= 0xFF00;
                    Cart->Mapper69.SquarePeriod[1] |= Value;
                } break;
                case 0x2:
                {
                    Cart->Mapper69.SquarePeriod[2] &= 0xFF00;
                    Cart->Mapper69.SquarePeriod[2] |= Value;
                } break;
                case 0x3:
                {
                    Cart->Mapper69.SquarePeriod[0] &= 0xFF;
                    Cart->Mapper69.SquarePeriod[0] |= (Value & 0xF) << 8;
                } break;
                case 0x4:
                {
                    Cart->Mapper69.SquarePeriod[1] &= 0xFF;
                    Cart->Mapper69.SquarePeriod[1] |= (Value & 0xF) << 8;
                } break;
                case 0x5:
                {
                    Cart->Mapper69.SquarePeriod[2] &= 0xFF;
                    Cart->Mapper69.SquarePeriod[2] |= (Value & 0xF) << 8;
                } break;
                case 0x6: break;
                case 0x7: Cart->Mapper69.ChannelEnable = Value; break;
                case 0x8: Cart->Mapper69.SquareVolume[0] = Value; break;
                case 0x9: Cart->Mapper69.SquareVolume[1] = Value; break;
                case 0xA: Cart->Mapper69.SquareVolume[2] = Value; break;
                case 0xB: break;
                case 0xC: break;
                case 0xD: break;
                case 0xE: break;
                case 0xF: break;
                default: break;
            }
        } break;
    }
}
