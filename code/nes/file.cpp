#define iNES_MAGIC 0x1a53454e
struct ines_header
{
    u32 Magic;
    u8 PrgRom;
    u8 ChrRom;
    u8 Flags6;
    u8 Flags7;
    u8 PrgRam;
    u8 Flags9;
    u8 Flags10;
    u8 Zero[5];
    u8 Data[1];
};

enum ines_flags6
{
    FLAGS6_MIRRORING  = 1 << 0,
    FLAGS6_BATTERYRAM = 1 << 1,
    FLAGS6_TRAINER    = 1 << 2,
    FLAGS6_FOURSCREEN = 1 << 3,
    FLAGS6_MAPPER_LOW = 0xF0,
};

enum ines_flags7
{
    FLAGS7_VSUNISYSTEM  = 1 << 0,
    FLAGS7_PLAYCHOICE10 = 1 << 1,
    FLAGS7_iNES2_0      = 3 << 2,
    FLAGS7_MAPPER_HI    = 0xF0,
};

enum ines_flags9
{
    FLAGS9_PAL  = 1 << 0,
};

enum ines_flags10
{
    FLAGS10_DUAL = 1 << 0,
    FLAGS10_PAL  = 1 << 1,
    FLAGS10_PRGRAM = 1 << 4,
    FLAGS10_BUSCONFLICT = 1 << 5,
};

static cart*
iNes_Load(string Filename, string Savename)
{
    mi BytesRead;
    u8 *Data = (u8 *)File_ReadEntireFile(Filename, &BytesRead);
    if (!Data) return 0;

    ines_header *Header = (ines_header *)Data;
    Data = Header->Data;

    cart *Result = 0;

    if (Header->Magic == iNES_MAGIC)
    {
        u16 Mapper = (Header->Flags6 >> 4) | (Header->Flags7 & 0xF0);

        // skip trainer
        if (Header->Flags6 & FLAGS6_TRAINER)
            Data += 512;

        b32 HasBattery = !!(Header->Flags6 & FLAGS6_BATTERYRAM);
        u8 Mirroring = !!(Header->Flags6 & FLAGS6_MIRRORING);

        mi PrgSize = Header->PrgRom * 0x4000;
        u8 *Prg = (u8 *)Api_Malloc(PrgSize);
        Memory_Copy(Prg, Data, PrgSize);
        Data += PrgSize;

        u8 ChrCount = MAX(1,Header->ChrRom);
        mi ChrSize = ChrCount * 0x2000;
        u8 *Chr = (u8 *)Api_Malloc(ChrSize);
        Memory_Copy(Chr, Data, ChrSize);
        Data += ChrSize;

        Result = (cart *)Api_Malloc(sizeof(cart));
        Result->PrgRom = Prg;
        Result->ChrRom = Chr;
        Result->PrgRomCount = Header->PrgRom;
        Result->ChrRomCount = ChrCount;
        Result->Mapper = Mapper;

        Result->PrgRam = (u8 *)Api_Malloc(0x2000);
        Result->PrgRamSize = 0x2000;

        if (HasBattery)
        {
            Result->Save = File_Open(Savename);
            if (File_Valid(Result->Save))
            {
                if (File_Size(Result->Save) == 0x2000)
                    File_ReadData(Result->Save, 0, Result->PrgRam, 0x2000);
                else
                {
                    File_Close(Result->Save);
                    Result->Save = File_OpenEmpty(Savename);
                }
            }
        }

        Result = Cart_Init(Result, Mirroring);
    }

    Api_Free(Header);
    return Result;
}



static gamepad_playback*
FM2_Load(string Filename)
{
    mi BytesRead;
    u8 *Data = (u8 *)File_ReadEntireFile(Filename, &BytesRead);
    if (!Data) return 0;
}
