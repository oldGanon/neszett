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
        case 0: { } break;
        case 1: { } break;
        case 2: { } break;
        case 3: { } break;
    }
}

static u8
Mapper5_RegisterRead(cart *Cart, u16 Address)
{
    switch (Address)
    {
        case 0x5204:
        {
            u8 Result = Cart->Mapper5.InFrame | Cart->Mapper5.IRQPending;
            Cart->Mapper5.IRQPending = 0;
            Console_ClearIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
            return Result;
        } break;

        case 0x5205: return Cart->Mapper5.Mul[0] * Cart->Mapper5.Mul[1]; break;
        case 0x5206: return ((u16)Cart->Mapper5.Mul[0] * (u16)Cart->Mapper5.Mul[1]) >> 8; break;
        default: break;
    }

    return 0;
}

static void
Mapper5_RegisterWrite(cart *Cart, u16 Address, u8 Value)
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
        case 0x5204: Cart->Mapper5.IRQEnable = Value & 0x80; 
                     if (Cart->Mapper5.IRQEnable && Cart->Mapper5.IRQPending)
                         Console_SetIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
                     break;
        case 0x5205: Cart->Mapper5.Mul[0] = Value; break;
        case 0x5206: Cart->Mapper5.Mul[1] = Value; break;

        default: break;
    }
}

static void
Mapper5_ReadMonitor(cart *Cart, u16 Address)
{
    if ((Address <= 0x2FFF) && (Address == Cart->Mapper5.IRQLastAddr))
    {
        if (++Cart->Mapper5.IRQAddrCount >= 2)
        {
            if (Cart->Mapper5.InFrame)
            {
                if (++Cart->Mapper5.Scanline == Cart->Mapper5.IRQScanline)
                {
                    Cart->Mapper5.IRQPending = 0x80;
                    if (Cart->Mapper5.IRQEnable)
                        Console_SetIRQ(Cart->Console, IRQ_SOURCE_MAPPER);
                }
            }
            else
            {
                Cart->Mapper5.InFrame = 0x40;
                Cart->Mapper5.Scanline = 0;
            }
        }
    }
    else Cart->Mapper5.IRQAddrCount = 0;
    Cart->Mapper5.IRQLastAddr = Address;
    Cart->Mapper5.PPUReading = 1;
}

static void
Mapper5_Step(cart *Cart)
{
    if (Cart->Mapper5.PPUReading)
        Cart->Mapper5.PPUIdleCount = 0;
    else if (++Cart->Mapper5.PPUIdleCount >= 3)
    {
        Cart->Mapper5.InFrame = 0;
        Cart->Mapper5.IRQLastAddr = 0;
    }
    Cart->Mapper5.PPUReading = 0;
}

static u8
Mapper5_Read(cart *Cart, u16 Address)
{
    switch (Address >> 13)
    {
        case 0: return Cart->Chr[Address >> 10][Address & 0x3FF];
        case 1:
        {
            Mapper5_ReadMonitor(Cart, Address);
            switch ((Cart->Mapper5.NametableMap >> ((Address >> 10) & 0x6)) & 0x3)
            {
                case 0: return Cart->NametableLo[Address & 0x3FF];
                case 1: return Cart->NametableHi[Address & 0x3FF];
                case 2: break;
                case 3: return Cart->Mapper5.FillModeTile;
                default: break;
            }
        } break;
        case 2: return Mapper5_RegisterRead(Cart, Address);
        case 3: if (Cart->Ram) { return Cart->Ram[(Address & 0x1FFF)]; } break;
        case 4: return Cart->Rom[0][Address & 0x1FFF];
        case 5: return Cart->Rom[1][Address & 0x1FFF];
        case 6: return Cart->Rom[2][Address & 0x1FFF];
        case 7: if ((Address == 0xFFFA) || (Address == 0xFFFB))
                {
                    Cart->Mapper5.InFrame = 0;
                    Cart->Mapper5.IRQLastAddr = 0;
                }
                return Cart->Rom[3][Address & 0x1FFF];
    }

    return 0;
}

static void
Mapper5_Write(cart *Cart, u16 Address, u8 Value)
{
    switch (Address >> 13)
    {
        case 0: Cart->Chr[Address >> 10][Address & 0x3FF] = Value; break;
        case 1:
        {
            switch ((Cart->Mapper5.NametableMap >> ((Address >> 10) & 0x6)) & 0x3)
            {
                case 0: Cart->NametableLo[Address & 0x3FF] = Value; break;
                case 1: Cart->NametableHi[Address & 0x3FF] = Value; break;
                case 2: break;
                case 3: break;
                default: break;
            }
        } break;
        case 2: Mapper5_RegisterWrite(Cart, Address, Value); break;
        case 3: if (Cart->Ram) { Cart->Ram[(Address & 0x1FFF)] = Value; } break;
        case 4: Cart->Rom[0][Address & 0x1FFF] = Value; break;
        case 5: Cart->Rom[1][Address & 0x1FFF] = Value; break;
        case 6: Cart->Rom[2][Address & 0x1FFF] = Value; break;
        case 7: break;
    }
}

static void
Mapper5_Init(cart *Cart)
{
    Cart->PrgRomCount <<= 1;
    Cart->ChrRomCount <<= 3;
    Cart->Read = Mapper5_Read;
    Cart->Write = Mapper5_Write;
    Cart->Step = Mapper5_Step;
    for (u8 i = 0; i < 8; ++i)
        Cart->Chr[i] = Cart->ChrRom + i * 0x400;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 4));
    Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 3));
    Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
}
