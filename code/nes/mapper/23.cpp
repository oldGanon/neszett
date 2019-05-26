
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

static void
Mapper23_Init(cart *Cart)
{
    Cart->PrgRomCount <<= 1;
    Cart->ChrRomCount <<= 3;
    Cart->Read = Mapper23_Read;
    Cart->Write = Mapper23_Write;
    Cart->Step = Mapper23_Step;
    for (u8 i = 0; i < 8; ++i)
        Cart->Chr[i] = Cart->ChrRom + i * 0x400;
    Cart->Ram = Cart->PrgRam;
    Cart->Rom[0] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[1] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[2] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 2));
    Cart->Rom[3] = Cart->PrgRom + (0x2000 * (Cart->PrgRomCount - 1));
}
