enum ppu_registers
{
    PPUCTRL   = 0x0,
    PPUMASK   = 0x1,
    PPUSTATUS = 0x2,
    OAMADDR   = 0x3,
    OAMDATA   = 0x4,
    PPUSCROLL = 0x5,
    PPUADDR   = 0x6,
    PPUDATA   = 0x7,
};

enum ppu_ctrl_flags
{
    PPU_NAMETABLE = 0x3,
    PPU_INCREMENT = 1 << 2,
    PPU_SPRTABLE  = 1 << 3,
    PPU_BGTABLE   = 1 << 4,
    PPU_SPRSIZE   = 1 << 5,
    PPU_MASTER    = 1 << 6,
    PPU_VBLANKNMI = 1 << 7,
};

enum ppu_mask_flags
{
    PPU_MASK_BLUE    = 1 << 7,
    PPU_MASK_GREEN   = 1 << 6,
    PPU_MASK_RED     = 1 << 5,
    PPU_MASK_DRAWSPR = 1 << 4,
    PPU_MASK_DRAWBG  = 1 << 3,
    PPU_MASK_SPRLEFT = 1 << 2,
    PPU_MASK_BGLEFT  = 1 << 1,
    PPU_MASK_GREY    = 1 << 0,
};

enum ppu_status_flags
{
    PPU_SPROVERFLOW = 1 << 5,
    PPU_SPRHIT      = 1 << 6,
    PPU_VBLANK      = 1 << 7,
};

struct ppu
{
    console *Console;
    
    u16 V;
    u16 T;
    u8 X;
    u8 W;

    u8 Controle;
    u8 Status;
    u8 Mask;
    u8 Buffer;

    u8 OAMAddr;
    u8 OAM[256];
    u8 Palettes[0x20];

    // render state
    u64 Tiles;
    b32 Odd;
    u16 Scanline;
    u16 Cycles;
    u8 NametableByte;
    u8 AttributeByte;
    u8 LowTile;
    u8 HighTile;
    u8 SpriteCount;
    u8 NMI;
    u8 SuppressVBL;
    u8 OAM2[32];
    u8 OAM3[32];
    u32 Sprites[8];
    u32 SpritesIndices[8];
    u8 Screen[240][260];
};

inline void
PPU_TriggerNMI(ppu *PPU)
{
    PPU->NMI = 14;
}

inline u8
PPU_ReadPalette(ppu *PPU, u16 Address)
{
    Address &= 0x1F;
    if (!(Address & 0x3)) Address = 0x0;
    u8 Color = PPU->Palettes[Address];
    if (PPU->Mask & 1) Color &= 0x30;
    return Color;
}

inline void
PPU_WritePalette(ppu *PPU, u16 Address, u8 Value)
{
    Address &= 0x1F;
    if (!(Address & 0xF)) Address = 0x0;
    PPU->Palettes[Address] = Value;
}

inline u8
PPU_ReadStatus(ppu *PPU)
{
    PPU->W = 0;
    u8 S = PPU->Status;
    PPU->Status &= ~PPU_VBLANK;
    if (PPU->Scanline == 241)
    {
        if (PPU->Cycles == 0)
            PPU->SuppressVBL = true;
        if (PPU->Cycles <= 2)
            PPU->NMI = 0;
    } 
    return S;
}

inline u8
PPU_ReadData(ppu *PPU)
{
    u16 Address = PPU->V & 0x3FFF;
    u8 Value = PPU->Buffer;
    PPU->Buffer = Cart_Read(PPU->Console, Address);

    if (Address >= 0x3F00)
        Value = PPU_ReadPalette(PPU, Address);

    PPU->V += (PPU->Controle & PPU_INCREMENT) ? 32 : 1;
    return Value;
}

inline u8
PPU_OAM(ppu *PPU)
{
    // if (Cycles <= 64 && 0 < Cycles && (Scanline <= 239 || Scanline == 261))
    //     return 0xFF
    return PPU->OAM[PPU->OAMAddr];
}

inline u8
PPU_ReadRegister(ppu *PPU, u16 Address)
{
    Address = (Address - 0x2000) & 0x7;
    switch (Address)
    {
        case PPUSTATUS: return PPU_ReadStatus(PPU); break;
        case OAMDATA:   return PPU_OAM(PPU); break;
        case PPUDATA:   return PPU_ReadData(PPU); break;
    }
    return 0;
}

inline void
PPU_WriteControle(ppu *PPU, u8 Value)
{
    if (!(Value & PPU_VBLANKNMI))
    {
        if (PPU->NMI > 12)
            PPU->NMI = 0;
    }
    else 
    {
        if (PPU->Scanline != 261 && 
            PPU->Cycles != 0 && 
            !(PPU->Controle & PPU_VBLANKNMI) &&
            PPU->Status & PPU_VBLANK)
                PPU_TriggerNMI(PPU);
    }
    PPU->Controle = Value;
    PPU->T &= ~0xC00;
    PPU->T |= ((u16)Value & 0x3) << 10;
}

inline void
PPU_WriteScroll(ppu *PPU, u8 Value)
{
    PPU->W ^= 0x1;
    if (PPU->W)
    {
        PPU->T &= ~0x1F;
        PPU->T |= Value >> 3;
        PPU->X = Value & 0x7;
    }
    else
    {
        PPU->T &= ~0x73E0;
        PPU->T |= (Value <<  2) & 0x03E0;
        PPU->T |= (Value << 12) & 0x7000;
    }
}

inline void
PPU_WriteAddress(ppu *PPU, u8 Value)
{
    PPU->W ^= 0x1;
    if (PPU->W)
    {
        PPU->T &= 0x00FF;
        PPU->T |= ((u16)Value << 8) & 0x3F00;
    }
    else
    {
        PPU->T &= 0xFF00;
        PPU->T |= Value & 0xFF;
        PPU->V = PPU->T;

        // NOTE: Put this somewhere in the mapper code somehow
        if (PPU->Console->Cart->Mapper == 4)
            Mapper4_IRQ(PPU->Console->Cart, (PPU->V >> 12) & 1);
    }
}

inline void
PPU_WriteData(ppu *PPU, u8 Value)
{
    u16 Address = PPU->V & 0x3FFF;
    if (Address < 0x3F00)
        Cart_Write(PPU->Console, Address, Value);
    else 
        PPU_WritePalette(PPU, Address, Value);

    PPU->V += (PPU->Controle & PPU_INCREMENT) ? 32 : 1;
}

inline void
PPU_WriteRegister(ppu *PPU, u16 Address, u8 Value)
{
    Address = (Address - 0x2000) & 0x7;
    switch (Address)
    {
        case PPUCTRL:   PPU_WriteControle(PPU, Value); break;
        case PPUMASK:   PPU->Mask = Value; break;
        case OAMADDR:   PPU->OAMAddr = Value; break;
        case OAMDATA:   PPU->OAM[PPU->OAMAddr++] = Value; break;
        case PPUSCROLL: PPU_WriteScroll(PPU, Value); break;
        case PPUADDR:   PPU_WriteAddress(PPU, Value); break;
        case PPUDATA:   PPU_WriteData(PPU, Value); break;
    }
}

inline void
PPU_FetchNametable(ppu *PPU)
{
    u16 Address = 0x2000 | (PPU->V & 0x0FFF);
    PPU->NametableByte = Cart_Read(PPU->Console, Address);
}

inline void
PPU_FetchAttribute(ppu *PPU)
{
    u16 V = PPU->V;
    u16 Address = 0x23C0 | (V & 0x0C00) | ((V >> 4) & 0x38) | ((V >> 2) & 0x07);
    PPU->AttributeByte = Cart_Read(PPU->Console, Address);
}

inline void
PPU_FetchTileLow(ppu *PPU)
{
    u16 T = (PPU->V >> 12) & 0x7;
    u16 P = 0 << 3;
    u16 RC = PPU->NametableByte << 4;
    u16 H = (PPU->Controle & PPU_BGTABLE) << 8;
    u16 Address = H | RC | P | T;
    PPU->LowTile = Cart_Read(PPU->Console, Address);
}

inline void
PPU_FetchTileHigh(ppu *PPU)
{
    u16 T = (PPU->V >> 12) & 0x7;
    u16 P = 1 << 3;
    u16 RC = PPU->NametableByte << 4;
    u16 H = (PPU->Controle & PPU_BGTABLE) << 8;
    u16 Address = H | RC | P | T;
    PPU->HighTile = Cart_Read(PPU->Console, Address);
}

inline u32
PPU_InterleavePattern(u32 L, u32 H, u32 A)
{
    L = (L | (L << 12)) & 0x000F000F;
    L = (L | (L <<  6)) & 0x03030303;
    L = (L | (L <<  3)) & 0x11111111;
    
    H = (H | (H << 12)) & 0x000F000F;
    H = (H | (H <<  6)) & 0x03030303;
    H = (H | (H <<  3)) & 0x11111111;
    H <<= 1;

    A |= A << 16;
    A |= A << 8;
    A |= A << 4;
    A <<= 2;

    return A | H | L;
}

inline void
PPU_ShiftRegisters(ppu *PPU)
{
    u16 Q = ((PPU->V >> 4) & 4) | (PPU->V & 2);
    u32 A = (PPU->AttributeByte >> Q) & 0x3;
    PPU->Tiles |= PPU_InterleavePattern(PPU->LowTile, PPU->HighTile, A);
}

inline u8
PPU_GetBackgroundPixel(ppu *PPU)
{
    u8 Tile = (PPU->Tiles >> (32 + ((7 - PPU->X) * 4))) & 0xF;
    return Tile;
}

inline u8
PPU_GetSpritePixel(ppu *PPU)
{
    u8 X = (PPU->Cycles - 1) & 0xFF;
    u16 Y = PPU->Scanline;
    for (u8 i = 0; i < 8; ++i)
    {
        u8 n = i * 4;
        u8 SprX = PPU->OAM3[n + 3];
        if (X < SprX) continue;
        SprX = X - SprX;
        if (SprX > 7) continue;

        if (!(PPU->OAM3[n + 2] & 0x40))
            SprX ^= 0x7;
        SprX <<= 2;
        u8 Color = (PPU->Sprites[i] >> SprX) & 0x0F;
        if (!(Color & 0x3)) continue;
        return Color | (i << 4);
    }
    return 0;
}

inline void
PPU_SetPixel(ppu *PPU, u16 X, u16 Y, u8 Pixel)
{
#if OPENGL_USETEXTUREBUFFER
    if (Y < 7 && 232 < Y) return;
    GlobalScreen[(Y-7) * 260 + X] = Pixel;
#else
    PPU->Screen[Y][X] = Pixel;
#endif
}

inline void
PPU_DrawPixel(ppu *PPU)
{
    u16 X = PPU->Cycles - 1;
    u16 Y = PPU->Scanline;
    
    u8 MaskBG = !(PPU->Mask & PPU_MASK_BGLEFT) && X < 8;
    u8 MaskSpr = !(PPU->Mask & PPU_MASK_SPRLEFT) && X < 8;
    u8 DrawBG = (!MaskBG && (PPU->Mask & PPU_MASK_DRAWBG));
    u8 DrawSpr = (!MaskSpr && (PPU->Mask & PPU_MASK_DRAWSPR));

    u8 Color;
    u8 BG = (DrawBG) ? PPU_GetBackgroundPixel(PPU) : 0;
    u8 Spr = (DrawSpr) ? PPU_GetSpritePixel(PPU) : 0;
    u8 Spri = Spr >> 4;
    if (!(Spr & 0x3))
        Color = BG;
    else if (!(BG & 0x3))
        Color = (Spr & 0x0F) | 0x10;
    else
    { 
        if (PPU->SpritesIndices[Spri] == 0 && X < 255 && Y != 0)
            PPU->Status |= PPU_SPRHIT;

        if (PPU->OAM3[Spri * 4 + 2] & 0x20)
            Color = BG;
        else
            Color = (Spr & 0x0F) | 0x10;
    }

    if (Y < 8 || 232 <= Y)
        PPU_SetPixel(PPU, X+1, Y, PPU->Palettes[0]);
    else
        PPU_SetPixel(PPU, X+1, Y, PPU_ReadPalette(PPU, Color));
}

inline u32
PPU_SpritePattern(ppu *PPU, u8 i)
{
    u16 Address;
    u8 Y = PPU->OAM2[i * 4];
    u8 Tile = PPU->OAM2[i * 4 + 1];
    u8 Attributes = PPU->OAM2[i * 4 + 2];
    u8 R = (PPU->Scanline - Y) & 0xFF;
    if (PPU->Controle & PPU_SPRSIZE)
    { // 8x16
        if (Attributes & 0x80) R = 15 - R;
        u16 H = (Tile & 0x1) << 12;
        u16 RC = (u16)((Tile & 0xFE) | (R >> 3)) << 4;
        R &= 0x7;
        Address = H | RC | R;
    }
    else
    { // 8x8
        if (Attributes & 0x80) R = 7 - R;
        u16 H = (PPU->Controle & PPU_SPRTABLE) << 9;
        u16 RC = (u16)(Tile & 0xFF) << 4;
        Address = H | RC | R;
    }
    u8 L = Cart_Read(PPU->Console, Address);
    u8 H = Cart_Read(PPU->Console, Address + 8);
    u8 A = Attributes & 0x3;
    return PPU_InterleavePattern(L, H, A);
}

inline void
PPU_SpriteFetch(ppu *PPU)
{
    for (u8 i = 0; i < 32; ++i)
        PPU->OAM3[i] = PPU->OAM2[i];

    for (u8 i = 0; i < 8; ++i)
        PPU->Sprites[i] = PPU_SpritePattern(PPU, i);
}

inline void
PPU_SpriteEvaluation(ppu *PPU, u8 i)
{
    if (i >= 64) return;

    u8 C = PPU->SpriteCount;
    u8 SpriteSize = (PPU->Controle & PPU_SPRSIZE) ? 16 : 8;
        
    u8 n = i * 4;
    u8 Y = PPU->OAM[n];
    u8 R = (PPU->Scanline - Y) & 0xFF;

    if (C >= 8)
    {
        if (Y > 0xEF) return;
        if (R >= SpriteSize) return;
        PPU->Status |= PPU_SPROVERFLOW;
    }
    else
    {   
        PPU->OAM2[C * 4] = Y;

        if (Y > 0xEF) return;
        if (R >= SpriteSize) return;

        PPU->OAM2[C * 4 + 1] = PPU->OAM[n + 1];
        PPU->OAM2[C * 4 + 2] = PPU->OAM[n + 2];
        PPU->OAM2[C * 4 + 3] = PPU->OAM[n + 3];
        PPU->SpritesIndices[C++] = i;
        PPU->SpriteCount = C;
    }
}

inline void
PPU_IncHoriV(ppu *PPU)
{
    if ((PPU->V & 0x001F) == 31)
    {
        PPU->V &= ~0x001F;
        PPU->V ^= 0x0400;        
    }
    else
        ++PPU->V;
}

inline void
PPU_IncVertV(ppu *PPU)
{
    if ((PPU->V & 0x7000) == 0x7000)
    {
        PPU->V &= ~0x7000;
        if ((PPU->V & 0x3A0) == 0x3A0)
        {
            if ((PPU->V & 0x3E0) == 0x3A0)
                PPU->V ^= 0x0800;
            PPU->V &= ~0x3E0;
        }
        else
            PPU->V += 0x20;
    }
    else
        PPU->V += 0x1000;
}

inline void
PPU_CopyVertV(ppu *PPU)
{
    PPU->V &= 0x841F;
    PPU->V |= PPU->T & 0x7BE0;
}

inline void
PPU_CopyHoriV(ppu *PPU)
{
    PPU->V &= 0xFBE0;
    PPU->V |= PPU->T & 0x041F;
}

inline void
PPU_EnterVBlank(ppu *PPU)
{
    Atomic_Inc(&GlobalFrame);

    if (!PPU->SuppressVBL)
    {
        PPU->Status |= PPU_VBLANK;
        if (PPU->Controle & PPU_VBLANKNMI)
            PPU_TriggerNMI(PPU);
    }
    PPU->SuppressVBL = false;

#if !(OPENGL_USETEXTUREBUFFER)
    if (PPU->Mask & (PPU_MASK_DRAWSPR | PPU_MASK_DRAWBG))
    {
        Memory_Copy(GlobalScreen, PPU->Screen[7], 260*226);
        Atomic_Set(&GlobalScreenChanged, 1);
    }
#endif
}

inline void
PPU_LeaveVBlank(ppu *PPU)
{
    PPU->Status &= ~(PPU_VBLANK | PPU_SPRHIT);

    u32 Phase = Atomic_Get(&GlobalPhase);
    if (PPU->Mask & (PPU_MASK_DRAWSPR | PPU_MASK_DRAWBG))
        Phase = Phase ^ 1;
    else
        Phase = ++Phase % 3;
    Atomic_Set(&GlobalPhase, Phase);
}

inline void
PPU_Step(ppu *PPU)
{
    if (PPU->NMI && --PPU->NMI == 0)
        Console_TriggerNMI(PPU->Console);

    b32 Draw = PPU->Mask & (PPU_MASK_DRAWSPR | PPU_MASK_DRAWBG);

    if (Draw && PPU->Odd && PPU->Scanline == 261 && PPU->Cycles == 337)
        ++PPU->Cycles;

    if (++PPU->Cycles > 340)
    {
        PPU->Cycles = 0;
        if (++PPU->Scanline > 261)
        {
            PPU->Scanline = 0;
            PPU->Odd ^= 1;
        }
    }

    b32 DrawLine = PPU->Scanline <= 239;
    b32 PreLine = PPU->Scanline == 261;
    b32 FetchLine = DrawLine || PreLine;
    b32 FetchCycle = (1 <= PPU->Cycles && PPU->Cycles <= 256) || (321 <= PPU->Cycles);

    if (Draw)
    {
        if (DrawLine)
        {
            if (PPU->Cycles == 0)
                PPU_SetPixel(PPU, 0, PPU->Scanline, PPU->Palettes[0]);
            else if (PPU->Cycles <= 256)
                PPU_DrawPixel(PPU);
            else if (PPU->Cycles <= 259)
                PPU_SetPixel(PPU, PPU->Cycles, PPU->Scanline, PPU->Palettes[0]);
            
            if (PPU->Cycles <= 1)
                PPU->SpriteCount = 0;
            else if (PPU->Cycles <= 64)
                PPU->OAM2[(PPU->Cycles - 1) >> 1] = 0xFF;
            else if (PPU->Cycles < 256)
            {
                u8 i = ((u8)PPU->Cycles - 114);
                if ((i % 2) == 0)
                    PPU_SpriteEvaluation(PPU, i / 2);
            }
        }

        if (FetchLine)
        {
            if (FetchCycle)
            {
                if (PPU->Cycles > 336)
                {
                    if (PPU->Cycles & 0x1)
                        PPU_FetchNametable(PPU);
                }
                else 
                {
                    PPU->Tiles <<= 4;
                    u8 Fetch = PPU->Cycles & 0x7;
                    switch (Fetch)
                    {
                        default: break;
                        case 1: PPU_FetchNametable(PPU); break;
                        case 3: PPU_FetchAttribute(PPU); break;
                        case 5: PPU_FetchTileLow(PPU); break;
                        case 7: PPU_FetchTileHigh(PPU); break;
                        case 0:
                        {
                            PPU_ShiftRegisters(PPU); 
                            PPU_IncHoriV(PPU);
                            if (PPU->Cycles == 256) 
                                PPU_IncVertV(PPU);
                        } break;
                    }
                }
            }
            else if (PPU->Cycles == 280)
            {
                PPU_SpriteFetch(PPU);
            }
            else if (PPU->Cycles == 257)
                PPU_CopyHoriV(PPU);
            else if (PreLine && 280 <= PPU->Cycles && PPU->Cycles <= 304)
                PPU_CopyVertV(PPU);
        }
    }
    
    if (!Draw && DrawLine)
        PPU_SetPixel(PPU, PPU->Cycles, PPU->Scanline, PPU->Palettes[0]);

    if (PPU->Scanline == 241 && PPU->Cycles == 1)
        PPU_EnterVBlank(PPU);
    else if (PreLine)
    {
        if (PPU->Cycles == 0)
            PPU->Status &= ~(PPU_SPRHIT | PPU_SPROVERFLOW);
        else if (PPU->Cycles == 1)
            PPU_LeaveVBlank(PPU);
    }
}

static void
PPU_Free(ppu *PPU)
{
    Api_Free(PPU);
}

static void
PPU_Reset(ppu *PPU)
{
    PPU->Controle = 0;
    PPU->Mask = 0;
    PPU->T = 0;
    PPU->X = 0;
    PPU->W = 0;
    PPU->Buffer = 0;

    PPU->Cycles = 340;
    PPU->Scanline = 240;
}

static void
PPU_Power(ppu *PPU)
{
    PPU->OAMAddr = 0;
    PPU->V = 0;
    PPU->Status = 0xBF;
    PPU_Reset(PPU);
}

static ppu*
PPU_Create(console *Console)
{
    ppu *PPU = (ppu *)Api_Malloc(sizeof(ppu));
    PPU->Console = Console;
    return PPU;
}