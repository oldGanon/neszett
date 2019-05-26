
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

static void
Mapper24_Init(cart *Cart)
{
    Cart->PrgRomCount <<= 1;
    Cart->ChrRomCount <<= 3;
    Cart->Read = Mapper24_Read;
    Cart->Write = Mapper24_Write;
    Cart->Step = Mapper24_Step;
    Cart->Audio = Mapper24_Audio;
    for (u8 i = 0; i < 8; ++i)
        Cart->Chr[i] = Cart->ChrRom + i * 0x400;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
}