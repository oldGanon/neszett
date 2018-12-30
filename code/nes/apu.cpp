
#define APU_DAC_HZ 43653
#define APU_DAC_CPU_HZ 41 // (CPU_HZ / APU_DAC_HZ)

#define APU_RUNNNING_AVG_LENGTH 512
#define APU_RUNNNING_AVG_NEW_FACTOR (1.0f / APU_RUNNNING_AVG_LENGTH)
#define APU_RUNNNING_AVG_OLD_FACTOR (1.0f - APU_RUNNNING_AVG_NEW_FACTOR)

enum apu_envelope_flags
{
    APU_ENVELOPE_DIVIDER = 0x0F,
    APU_ENVELOPE_DISABLE = 0x10,
    APU_ENVELOPE_LOOP = 0x20,
    APU_ENVELOPE_WRITE = 0x80,
};

struct apu_envelope
{
    u8 Flags;
    u8 Counter;
    u8 Volume;
    u8 Write;
};

struct apu_sweep
{
    u8 Counter;
    u8 Flags;
    u8 Write;
};

struct apu_timer
{
    u16 Counter;
    u16 Period;
};

struct apu_duty
{
    u8 Mode;
    u8 Counter;
};

struct apu_square
{
    apu_envelope Envelope;
    apu_timer Timer;
    apu_sweep Sweep;
    apu_duty Duty;
    u8 Length;
    u8 Enabled;
};

struct apu_triangle
{
    apu_timer Timer;
    u8 Envelope;
    u8 Counter;
    u8 Halt;
    u8 Length;
    u8 Sequence;
    u8 Enabled;
};

struct apu_noise
{
    apu_envelope Envelope;
    apu_timer Timer;
    u16 Shift;
    u8 Length;
    u8 Mode;
    u8 Enabled;
};

struct apu_dmc
{
    u16 Address;
    u16 CurrAddress;
    u8 Length;
    u8 CurrLength;
    u8 Period;
    u8 Counter;
    u8 Flags;
    u8 Volume;
    u8 BitCount;
    u8 Samples;
    u8 Enabled;
};

struct apu
{
    console *Console;

    apu_square Square[2];
    apu_triangle Triangle;
    apu_noise Noise;
    apu_dmc DMC;

    u8 Even;

    u8 Status;
    u8 FramCounter;

    u16 Sequencer;

    u16 DAC;
    f32 RunningAverage;
};

static u8 APU_LengthLUT[32] =
{
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
    0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
    0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E,
};

static u8 APU_DutyLUT[4][8] = { 
    {0,1,0,0,0,0,0,0},
    {0,1,1,0,0,0,0,0},
    {0,1,1,1,1,0,0,0},
    {1,0,0,1,1,1,1,1},
};

static u8 APU_Triangle[32] = {
    0xF,0xE,0xD,0xC,0xB,0xA,0x9,0x8,0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0,
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
};

static u16 APU_NoiseLUT[16] = {
    0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
    0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4,
};

static u8 APU_DMCLUT[16] = {
    214, 190, 170, 160, 143, 127, 113, 107, 95, 80, 71, 64, 53, 42, 36, 27,
};

//
// OUTPUT
//

static u8
APU_SquareOut(apu_square *Square)
{
    if (!Square->Length) return 0;
    if (!APU_DutyLUT[Square->Duty.Mode][Square->Duty.Counter]) return 0;
    if (Square->Timer.Period < 8 || Square->Timer.Period > 0x7FF) return 0;
    if (Square->Envelope.Flags & APU_ENVELOPE_DISABLE) return (Square->Envelope.Flags & APU_ENVELOPE_DIVIDER);
    return Square->Envelope.Volume;
}

static u8
APU_TriangleOut(apu_triangle *Triangle)
{
    // if (!Triangle->Length) return 7;
    // if (!Triangle->Counter) return 7;
    if (Triangle->Timer.Period < 2) return 7;
    return APU_Triangle[Triangle->Sequence & 0x1F];
}

static u8
APU_NoiseOut(apu_noise *Noise)
{
    if (!Noise->Length) return 0;
    if (Noise->Shift & 1) return 0;
    if (Noise->Envelope.Flags & APU_ENVELOPE_DISABLE) return (Noise->Envelope.Flags & APU_ENVELOPE_DIVIDER);
    return Noise->Envelope.Volume;
}

static u8
APU_DMCOut(apu_dmc *DMC)
{
    return DMC->Volume;
}

static void
APU_SampleOut(apu *APU)
{
    f32 SquareOut = (f32)APU_SquareOut(APU->Square + 0) + (f32)APU_SquareOut(APU->Square + 1);
    if (SquareOut > 0.0f) SquareOut = 95.88f / (8128.0f / SquareOut + 100.0f);

    f32 TNDOut = APU_TriangleOut(&APU->Triangle) / 8227.0f;
    TNDOut += APU_NoiseOut(&APU->Noise) / 12241.0f;
    TNDOut += APU_DMCOut(&APU->DMC) / 22638.0f;
    if (TNDOut > 0.0f) TNDOut = 159.79f / (1.0f / TNDOut + 100.0f);
    
    f32 MapperOut = Cart_Audio(APU->Console->Cart);

    f32 Out = (SquareOut + TNDOut + MapperOut) * 0.25f;
    f32 NewAverage = APU->RunningAverage*APU_RUNNNING_AVG_OLD_FACTOR + Out*APU_RUNNNING_AVG_NEW_FACTOR;
    Out -= APU->RunningAverage;
    APU->RunningAverage = NewAverage;

    SDL_QueueAudio(GlobalAudioDevice, &Out, 4);
    APU->DAC = 0;
}

//
// SQUARE
//

static void
APU_StepSweep(apu *APU)
{
    for (u8 i = 0; i < 2; ++i)
    {
        apu_square *Square = APU->Square + i;
        if (Square->Sweep.Write)
            Square->Sweep.Write = 0;
        else if (Square->Sweep.Counter > 0)
        {
            --Square->Sweep.Counter;
            continue;
        }

        Square->Sweep.Counter = (Square->Sweep.Flags >> 4) & 0x7;

        if (!(Square->Sweep.Flags & 0x80))
            continue;

        if (Square->Timer.Period < 8 || Square->Timer.Period > 0x7FF)
            continue;

        u8 Shift = (Square->Sweep.Flags & 0x07);
        u8 Negate = (Square->Sweep.Flags & 0x08);
        u16 Period = Square->Timer.Period >> Shift;
        if (Negate) 
        {
            Period ^= 0xFFFF;
            if (i) ++Period;
        }
        Square->Timer.Period += Period;
    }
}

static void
APU_TimerClock(apu_square *Square)
{
    Square->Duty.Counter = (Square->Duty.Counter + 1) & 0x7;
}

static void
APU_WriteEnvelope(apu_square *Square, u8 Value)
{
    // value 4 bad (example smb1 death)
    // if (Value == 4) return;

    Square->Duty.Mode = Value >> 6;
    Square->Envelope.Flags = Value & 0x3F;
}

static void
APU_WriteSweep(apu_square *Square, u8 Value)
{
    Square->Sweep.Write = 0xFF;
    Square->Sweep.Flags = Value;
}

static void
APU_WriteTimer(apu_square *Square, u8 Value)
{
    Square->Timer.Period &= 0xFF00;
    Square->Timer.Period |= Value;
}

static void
APU_WriteLength(apu_square *Square, u8 Value)
{
    if (Square->Enabled)
        Square->Length = APU_LengthLUT[Value >> 3];
    Square->Timer.Period &= 0xFF;
    Square->Timer.Period |= ((u16)Value & 7) << 8;
    Square->Envelope.Write = 0xFF;
}

//
// Triangle
//

static void
APU_StepTriangle(apu *APU)
{
    if (APU->Triangle.Halt)
        APU->Triangle.Counter = APU->Triangle.Envelope & 0x7F;
    else if (APU->Triangle.Counter > 0)
        --APU->Triangle.Counter;
    if (!(APU->Triangle.Envelope & 0x80))
        APU->Triangle.Halt = 0;
}

static void
APU_TimerClock(apu_triangle *Triangle)
{
    if (Triangle->Length && Triangle->Counter)
        ++Triangle->Sequence;
}

static void
APU_WriteEnvelope(apu_triangle *Triangle, u8 Value)
{
    Triangle->Envelope = Value;
}

static void
APU_WriteTimer(apu_triangle *Triangle, u8 Value)
{
    Triangle->Timer.Period &= 0xFF00;
    Triangle->Timer.Period |= Value;
}

static void
APU_WriteLength(apu_triangle *Triangle, u8 Value)
{
    if (Triangle->Enabled)
        Triangle->Length = APU_LengthLUT[Value >> 3];
    Triangle->Timer.Period &= 0xFF;
    Triangle->Timer.Period |= ((u16)Value & 7) << 8;
    Triangle->Timer.Counter = Triangle->Timer.Period;
    Triangle->Halt = 1;
}

//
// NOISE
//

static void
APU_TimerClock(apu_noise *Noise)
{
    u16 Bit = Noise->Shift;
    Noise->Shift >>= 1;
    u16 Shift = Noise->Shift;
    if (Noise->Mode) Shift >>= 5;
    Bit ^= Noise->Shift;
    Bit &= 1;
    Bit <<= 14;
    Noise->Shift |= Bit;
}

static void
APU_WriteEnvelope(apu_noise *Noise, u8 Value)
{
    Noise->Envelope.Flags = Value & 0x3F;
}

static void
APU_WriteMode(apu_noise *Noise, u8 Value)
{
    Noise->Mode = Value >> 7;
    Noise->Timer.Period = APU_NoiseLUT[Value & 0xF];
}

static void
APU_WriteLength(apu_noise *Noise, u8 Value)
{
    if (Noise->Enabled)
        Noise->Length = APU_LengthLUT[Value >> 3];
    Noise->Envelope.Write = 0xFF;
}

//
// DMC
//

static void
APU_WriteMode(apu_dmc *DMC, console *Console, u8 Value)
{
    if (!(Value & 0x80))
        Console_ClearIRQ(Console, IRQ_SOURCE_DMC);
    DMC->Flags = Value & 0xC0;
    DMC->Period = APU_DMCLUT[Value&0xF];
}

static void
APU_WriteDAC(apu_dmc *DMC, u8 Value)
{
    DMC->Volume = Value & 0x7F;
}

static void
APU_WriteAddress(apu_dmc *DMC, u8 Value)
{
    DMC->Address = ((u16)Value << 6) | 0xC000;
}

static void 
APU_WriteLength(apu_dmc *DMC, u8 Value)
{
    if (DMC->Enabled)
        DMC->Length = ((u16)Value << 4) | 1;
}

static void 
APU_Reset(apu_dmc *DMC)
{
    DMC->CurrLength = DMC->Length;
    DMC->CurrAddress = DMC->Address;
}

static void
APU_TimerStep(apu_dmc *DMC, console *Console)
{
    if (!DMC->Length) return;
    if (DMC->CurrLength && DMC->BitCount == 0)
    {
        Console->CPU->Busy += 4;
        DMC->Samples = Cart_Read(Console, DMC->CurrAddress++);
        DMC->BitCount = 8;
        if (!DMC->CurrAddress) DMC->CurrAddress = 0x8000;
        if (--DMC->CurrLength == 0)
        {
            if (DMC->Flags & 0x40)
                APU_Reset(DMC);
            else if (DMC->Flags & 0x80)
            {
                Console->APU->FramCounter |= 0x80;
                Console_SetIRQ(Console, IRQ_SOURCE_DMC);
            }
        }
    }
    if (--DMC->Counter == 0)
    {
        DMC->Counter = DMC->Period;
        if (!DMC->BitCount) return;
        if (DMC->Samples&1)
        {
            if (DMC->Volume < 126)
                DMC->Volume += 2;
        }
        else
        {
            if (DMC->Volume > 1) 
                DMC->Volume -= 2;

        }
        DMC->Samples >>= 1;
        --DMC->BitCount;
    }
}

//
// STEP
//

static void
APU_StepEnvelope(apu_envelope *Envelope)
{
    if (Envelope->Write)
    {
        Envelope->Write = 0;
        Envelope->Volume = 0xF;
        Envelope->Counter = (Envelope->Flags & APU_ENVELOPE_DIVIDER);
        return;
    }

    if (Envelope->Counter-- == 0)
    {
        Envelope->Counter = (Envelope->Flags & APU_ENVELOPE_DIVIDER);

        if (Envelope->Volume > 0)
            Envelope->Volume--;
        else if (Envelope->Flags & APU_ENVELOPE_LOOP)
            Envelope->Volume = 0xF;
    }
}

static void
APU_StepEnvelope(apu *APU)
{
    APU_StepEnvelope(&APU->Square[0].Envelope);
    APU_StepEnvelope(&APU->Square[1].Envelope);
    APU_StepEnvelope(&APU->Noise.Envelope);
}

static void
APU_StepLength(apu *APU)
{
    if (!(APU->Square[0].Envelope.Flags & 0x20) && APU->Square[0].Length)
        --APU->Square[0].Length;
    if (!(APU->Square[1].Envelope.Flags & 0x20) && APU->Square[1].Length)
        --APU->Square[1].Length;
    if (!(APU->Triangle.Envelope & 0x80) && APU->Triangle.Length)
        --APU->Triangle.Length;
    if (!(APU->Noise.Envelope.Flags & 0x20) && APU->Noise.Length)
        --APU->Noise.Length;
}

static void
APU_IRQ(apu *APU)
{
    if (!(APU->FramCounter & 0x40))
    {
        APU->Status |= 0x40;
        Console_SetIRQ(APU->Console, IRQ_SOURCE_APU);
    }
}

static void
APU_SequencerStep(apu *APU)
{
    u8 Mode = (APU->FramCounter & 0x80) ? 5 : 4;
    if (Mode == 4)
    {
        switch (APU->Sequencer++)
        {
            case 29828: APU_IRQ(APU); break;
            case 29829: APU->Sequencer = 0;
            case 14913: APU_StepLength(APU);
                        APU_StepSweep(APU);
            case  7457: 
            case 22371: APU_StepEnvelope(APU);
                        APU_StepTriangle(APU);
        }
    }
    else
    {
        switch (APU->Sequencer++)
        {
            case 37282: APU->Sequencer = 0; break;
            case  7457:
            case 22371: APU_StepLength(APU);
                        APU_StepSweep(APU);
            case 14913:
            case 37281: APU_StepEnvelope(APU);
                        APU_StepTriangle(APU);
            case 29828: break;
        }
    }
}

static b32
APU_TimerStep(apu_timer *Timer)
{
    if (Timer->Counter > 0)
    {
        Timer->Counter--;
        return false;
    }
    else
    {
        Timer->Counter = Timer->Period;
        return true;
    }
}

static void
APU_TimerStep(apu *APU)
{
    APU->Even ^= 1;
    if (APU->Even)
    {
        if (APU_TimerStep(&APU->Square[0].Timer))
            APU_TimerClock(APU->Square + 0);
        if (APU_TimerStep(&APU->Square[1].Timer))
            APU_TimerClock(APU->Square + 1);
        if (APU_TimerStep(&APU->Noise.Timer))
            APU_TimerClock(&APU->Noise);
        APU_TimerStep(&APU->DMC, APU->Console);
    }
    if (APU_TimerStep(&APU->Triangle.Timer))
        APU_TimerClock(&APU->Triangle);
}

static void
APU_Step(apu *APU)
{
    APU_SequencerStep(APU);
    
    APU_TimerStep(APU);

    if (++APU->DAC >= APU_DAC_CPU_HZ)
        APU_SampleOut(APU);
}

//
// WRITE
//

inline void
APU_WriteStatus(apu *APU, u8 Value)
{
    APU->Status &= 0x40;
    APU->Status |= Value & 0x1f;

    APU->Square[0].Enabled = Value & 0x01;
    APU->Square[1].Enabled = Value & 0x02;
    APU->Triangle.Enabled = Value & 0x04;
    APU->Noise.Enabled = Value & 0x08;
    APU->DMC.Enabled = Value & 0x10;
    if (!APU->Square[0].Enabled) APU->Square[0].Length = 0;
    if (!APU->Square[1].Enabled) APU->Square[1].Length = 0;
    if (!APU->Triangle.Enabled)  APU->Triangle.Length = 0;
    if (!APU->Noise.Enabled)     APU->Noise.Length = 0;
    if (!APU->DMC.Enabled)       APU->DMC.Length = 0;
    else APU_Reset(&APU->DMC);
}

inline void
APU_WriteFrameCounter(apu *APU, u8 Value)
{
    if (APU->Console->CPU->Cycles & 1)
        APU->Sequencer = 0xFFFD;
    else
        APU->Sequencer = 0xFFFE;

    APU->FramCounter = Value;
    if (APU->FramCounter & 0x80)
    {
        APU_StepLength(APU);
        APU_StepSweep(APU);
        APU_StepEnvelope(APU);
        APU_StepTriangle(APU);
    }
    if (APU->FramCounter & 0x40)
    {
        APU->Status &= ~0x40;
        Console_ClearIRQ(APU->Console, IRQ_SOURCE_APU);
    }
}

static void
APU_WriteRegister(apu *APU, u16 Address, u8 Value)
{
    switch (Address)
    {
        case 0x4000: APU_WriteEnvelope(APU->Square + 0, Value); break;
        case 0x4001: APU_WriteSweep(APU->Square + 0, Value); break;
        case 0x4002: APU_WriteTimer(APU->Square + 0, Value); break;
        case 0x4003: APU_WriteLength(APU->Square + 0, Value); break;
        
        case 0x4004: APU_WriteEnvelope(APU->Square + 1, Value); break;
        case 0x4005: APU_WriteSweep(APU->Square + 1, Value); break;
        case 0x4006: APU_WriteTimer(APU->Square + 1, Value); break;
        case 0x4007: APU_WriteLength(APU->Square + 1, Value); break;

        case 0x4008: APU_WriteEnvelope(&APU->Triangle, Value); break;
        case 0x4009: break;
        case 0x400A: APU_WriteTimer(&APU->Triangle, Value); break;
        case 0x400B: APU_WriteLength(&APU->Triangle, Value); break;
        
        case 0x400C: APU_WriteEnvelope(&APU->Noise, Value); break;
        case 0x400D: break;
        case 0x400E: APU_WriteMode(&APU->Noise, Value); break;
        case 0x400F: APU_WriteLength(&APU->Noise, Value); break;

        case 0x4010: APU_WriteMode(&APU->DMC, APU->Console, Value); break;
        case 0x4011: APU_WriteDAC(&APU->DMC, Value); break;
        case 0x4012: APU_WriteAddress(&APU->DMC, Value); break;
        case 0x4013: APU_WriteLength(&APU->DMC, Value); break;

        case 0x4015: APU_WriteStatus(APU, Value); break;
        
        case 0x4017: APU_WriteFrameCounter(APU, Value); break;
    }
}

//
// READ
//

static u8
APU_ReadStatuts(apu *APU)
{
    u8 S = APU->Status & 0xC0;
    
    APU->Status &= ~0x40;
    Console_ClearIRQ(APU->Console, IRQ_SOURCE_APU);

    if (APU->Square[0].Length) S |= 0x01;
    if (APU->Square[1].Length) S |= 0x02;
    if (APU->Triangle.Length)  S |= 0x04;
    if (APU->Noise.Length)     S |= 0x08;
    if (APU->DMC.Length)       S |= 0x10;
    return S;
}

static u8
APU_ReadRegister(apu *APU, u16 Address)
{
    if (Address == 0x4015)
        return APU_ReadStatuts(APU);
    else return 0;
}

//
// APU
//

static void
APU_Free(apu *APU)
{
    Api_Free(APU);
}

static void
APU_Reset(apu *APU)
{
    APU->Even = 0;
    APU->Status = 0;
    APU_WriteRegister(APU, 0x4015, 0);
    // for (u8 i = 0; i < 0xF; ++i)
    //     APU_WriteRegister(APU, 0x4000 + i, 0);

    APU->RunningAverage = 159.79f / (1.0f / (1.5f / 8227.0f) + 100.0f);
}

static void
APU_Power(apu *APU)
{
    APU->FramCounter = 0;
    APU->Noise.Shift = 1;
    APU_Reset(APU);
}

static apu*
APU_Create(console *Console)
{
    apu *APU = (apu *)Api_Malloc(sizeof(apu));
    APU->Console = Console;
    return APU;
}
