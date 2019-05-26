
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

static void
Mapper69_Init(cart *Cart)
{
    Cart->PrgRomCount <<= 1;
    Cart->ChrRomCount <<= 3;
    Cart->Read = Mapper69_Read;
    Cart->Write = Mapper69_Write;
    Cart->Step = Mapper69_Step;
    Cart->Audio = Mapper69_Audio;
    for (u8 i = 0; i < 8; ++i)
        Cart->Chr[i] = Cart->ChrRom + i * 0x400;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
}