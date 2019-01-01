struct cpu
{
    console *Console;
    u64 Cycles;
    u64 Busy;
    u8 NMIOccurred;
    u8 IRQOccurred;
    u8 IRQDelayed;

    u16 PC; // Program Counter
    u8 AC; // Accumulator
    u8 X;
    u8 Y;
    u8 SP; // Stack Pointer
    u8 SR; // Status Register
    u8 __PADDING;
    u8 RAM[0x800];
};

enum status_flags
{
    STATUS_NEGATIVE  = 1 << 7,
    STATUS_OVERFLOW  = 1 << 6,
    STATUS_          = 1 << 5,
    STATUS_BREAK     = 1 << 4,
    STATUS_DECIMAL   = 1 << 3,
    STATUS_INTERRUPT = 1 << 2,
    STATUS_ZERO      = 1 << 1,
    STATUS_CARRY     = 1 << 0,
};

inline void
CPU_DMA(cpu *CPU, u8 Value)
{
    u16 Address = (u16)(Value & 0x7) << 8;
    const u8 *Src = CPU->RAM + Address;
    for (u32 i = 0; i < 256; ++i)
        PPU_WriteRegister(CPU->Console, OAMDATA, Src[i]);
    CPU->Busy += (CPU->Cycles & 1) ? 514 : 513;
}

inline u8
CPU_Read(cpu *CPU, u16 Address)
{
    if (Address < 0x2000)
        return CPU->RAM[Address & 0x7FF];
    else if (Address < 0x4000)
        return PPU_ReadRegister(CPU->Console, Address);
    else if (Address < 0x4020)
        if (Address <= 0x4013 || Address == 0x4015)
            return APU_ReadRegister(CPU->Console, Address);
        else if (Address <= 0x4017)
            return Gamepad_Read(CPU->Console, Address & 1);
        else return 0;
    else if (Address < 0x6000)
        return 0; // unused
    else return Cart_Read(CPU->Console, Address);
}

inline void
CPU_Write(cpu *CPU, u16 Address, u8 Value)
{
    if (Address < 0x2000)
        CPU->RAM[Address & 0x7FF] = Value;
    else if (Address < 0x4000)
        PPU_WriteRegister(CPU->Console, Address, Value);
    else if (Address < 0x4020)
        if (Address <= 0x4013 || Address == 0x4015 || Address == 0x4017)
            APU_WriteRegister(CPU->Console, Address, Value);
        else if (Address == 0x4016)
            Gamepad_Write(CPU->Console, Value);
        else if (Address == 0x4014) CPU_DMA(CPU, Value);
        else return;
    else if (Address < 0x6000); // unused
    else Cart_Write(CPU->Console, Address, Value);
}

inline u8 CPU_NextByte(cpu *CPU) { return CPU_Read(CPU, CPU->PC++); }
inline i8 CPU_NextSByte(cpu *CPU) { return CPU_Read(CPU, CPU->PC++); }
inline u16 CPU_NextWord(cpu *CPU) { return CPU_NextByte(CPU) | ((u16)CPU_NextByte(CPU) << 8); }
inline u16 CPU_ReadWord(cpu *CPU, u16 Address) { return CPU_Read(CPU, Address) | ((u16)CPU_Read(CPU, Address+1) << 8); }
inline u16 CPU_ReadWordWrap(cpu *CPU, u16 A) { return CPU_Read(CPU,A)|((u16)CPU_Read(CPU,(A&0xFF00)|((A+1)&0xFF)) << 8); }

inline void CPU_UpdateFlagsNZ(cpu *CPU, u8 Result)
{
    CPU->SR &= ~(STATUS_ZERO | STATUS_NEGATIVE);
    if (Result == 0) CPU->SR |= STATUS_ZERO;
    if (Result & 0x80) CPU->SR |= STATUS_NEGATIVE;
}

inline void CPU_Push(cpu *CPU, u8 Value)
{
    CPU->RAM[CPU->SP-- | 0x100] = Value;
}

inline u8 CPU_Pull(cpu *CPU)
{
    return CPU->RAM[++CPU->SP | 0x100];
}

// Add Memory to Accumulator with Carry
inline void ADC(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    u16 Intermediate = (u16)Value + (u16)(CPU->SR & STATUS_CARRY);
    Intermediate = (u16)CPU->AC + Intermediate;
    u8 AC = Intermediate & 0xFF;

    CPU->SR &= ~(STATUS_CARRY | STATUS_OVERFLOW | STATUS_ZERO | STATUS_NEGATIVE);
    if (AC == 0) CPU->SR |= STATUS_ZERO;
    if (AC & 0x80) CPU->SR |= STATUS_NEGATIVE;
    if (Intermediate > 255)
        CPU->SR |= STATUS_CARRY;
    if ((CPU->AC ^ AC) & (Value ^ AC) & 0x80)
        CPU->SR |= STATUS_OVERFLOW;

	CPU->AC = AC;
}

// Bitwise And Value to Accumulator
inline void AND(cpu *CPU, u16 Address)
{
    CPU->AC &= CPU_Read(CPU, Address);
    CPU_UpdateFlagsNZ(CPU, CPU->AC);
}

// Shift Memory Left One Bit
inline void ASL(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    CPU->SR &= ~(STATUS_ZERO | STATUS_NEGATIVE | STATUS_CARRY);
    CPU->SR |= Value >> 7;
    Value <<= 1;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU_Write(CPU, Address, Value);
}

// Shift Accumulator Left One Bit
inline void ASLA(cpu *CPU, u16 Address)
{
    u8 Value = CPU->AC;
    CPU->SR &= ~(STATUS_ZERO | STATUS_NEGATIVE | STATUS_CARRY);
    CPU->SR |= Value >> 7;
    Value <<= 1;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU->AC = Value;
}

// Test Bits in Value with Accumulator
inline void BIT(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    CPU->SR &= ~(STATUS_ZERO | STATUS_NEGATIVE | STATUS_OVERFLOW);
    CPU->SR |= Value & (STATUS_NEGATIVE | STATUS_OVERFLOW);
    if ((CPU->AC & Value) == 0) CPU->SR |= STATUS_ZERO;
}

inline b32 CPU_PageCross(u16 A, u16 B)
{
    return ((A & 0xFF00) != (B & 0xFF00));
}

inline void
CPU_Branch(cpu *CPU, u16 Address)
{
    ++CPU->Busy;
    if (CPU_PageCross(CPU->PC, Address))
        ++CPU->Busy;
    CPU->PC = Address;
}

// Branch on Status Flag Set
inline void BCS(cpu *CPU, u16 Address) { if (  CPU->SR & STATUS_CARRY)    CPU_Branch(CPU, Address); }
inline void BEQ(cpu *CPU, u16 Address) { if (  CPU->SR & STATUS_ZERO)     CPU_Branch(CPU, Address); }
inline void BMI(cpu *CPU, u16 Address) { if (  CPU->SR & STATUS_NEGATIVE) CPU_Branch(CPU, Address); }
inline void BVS(cpu *CPU, u16 Address) { if (  CPU->SR & STATUS_OVERFLOW) CPU_Branch(CPU, Address); }

// Branch on Status Flag Clear
inline void BCC(cpu *CPU, u16 Address) { if (!(CPU->SR & STATUS_CARRY))    CPU_Branch(CPU, Address); }
inline void BNE(cpu *CPU, u16 Address) { if (!(CPU->SR & STATUS_ZERO))     CPU_Branch(CPU, Address); }
inline void BPL(cpu *CPU, u16 Address) { if (!(CPU->SR & STATUS_NEGATIVE)) CPU_Branch(CPU, Address); }
inline void BVC(cpu *CPU, u16 Address) { if (!(CPU->SR & STATUS_OVERFLOW)) CPU_Branch(CPU, Address); }

// Force Break
inline void BRK(cpu *CPU, u16 Address)
{
    ++CPU->PC;
    CPU_Push(CPU, CPU->PC >> 8);
    CPU_Push(CPU, CPU->PC & 0xFF);
    CPU_Push(CPU, CPU->SR | 0x30);
    CPU->PC = CPU_ReadWord(CPU, 0xFFFE);
    CPU->SR |= STATUS_INTERRUPT;
}

// Clear Flag
inline void CLC(cpu *CPU, u16 Address) { CPU->SR &= ~(STATUS_CARRY); }
inline void CLD(cpu *CPU, u16 Address) { CPU->SR &= ~(STATUS_DECIMAL); }
inline void CLI(cpu *CPU, u16 Address) { if (CPU->SR & STATUS_INTERRUPT) { CPU->SR &= ~(STATUS_INTERRUPT); CPU->IRQDelayed = CPU->IRQOccurred; } }
inline void CLV(cpu *CPU, u16 Address) { CPU->SR &= ~(STATUS_OVERFLOW); }

inline void CPU_Compare(cpu *CPU, u8 Register, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    CPU->SR &= ~(STATUS_ZERO | STATUS_NEGATIVE | STATUS_CARRY);
    if (Register >= Value) CPU->SR |= STATUS_CARRY;
    if (Register == Value) CPU->SR |= STATUS_ZERO;
    if ((u8)(Register - Value) & 0x80) CPU->SR |= STATUS_NEGATIVE;
}

// Compare Value with Register
inline void CMP(cpu *CPU, u16 Address) { CPU_Compare(CPU, CPU->AC, Address); }
inline void CPX(cpu *CPU, u16 Address) { CPU_Compare(CPU, CPU->X, Address); }
inline void CPY(cpu *CPU, u16 Address) { CPU_Compare(CPU, CPU->Y, Address); }

// Decrement Memory by One
inline void DEC(cpu *CPU, u16 Address) { u8 Value = CPU_Read(CPU, Address); CPU_Write(CPU, Address, --Value); CPU_UpdateFlagsNZ(CPU, Value); }
inline void DEX(cpu *CPU, u16 Address) { CPU_UpdateFlagsNZ(CPU, --CPU->X); }
inline void DEY(cpu *CPU, u16 Address) { CPU_UpdateFlagsNZ(CPU, --CPU->Y); }

inline void EOR(cpu *CPU, u16 Address) { CPU->AC ^= CPU_Read(CPU, Address); CPU_UpdateFlagsNZ(CPU, CPU->AC); }

// Increment Memory by One
inline void INC(cpu *CPU, u16 Address) { u8 Value = CPU_Read(CPU, Address); CPU_Write(CPU, Address, ++Value); CPU_UpdateFlagsNZ(CPU, Value); }
inline void INX(cpu *CPU, u16 Address) { CPU_UpdateFlagsNZ(CPU, ++CPU->X); }
inline void INY(cpu *CPU, u16 Address) { CPU_UpdateFlagsNZ(CPU, ++CPU->Y); }

// Jump
inline void JMP(cpu *CPU, u16 Address)
{
    CPU->PC = Address;
}

static u8 CallStack = 0;

// Jump Saving Return Address
inline void JSR(cpu *CPU, u16 Address)
{
    --CPU->PC;
    CPU_Push(CPU, CPU->PC >> 8);
    CPU_Push(CPU, CPU->PC & 0xFF);
    CPU->PC = Address;
}

// Load Register with Value
inline void LDA(cpu *CPU, u16 Address) { CPU->AC = CPU_Read(CPU, Address); CPU_UpdateFlagsNZ(CPU, CPU->AC); }
inline void LDX(cpu *CPU, u16 Address) { CPU->X = CPU_Read(CPU, Address); CPU_UpdateFlagsNZ(CPU, CPU->X); }
inline void LDY(cpu *CPU, u16 Address) { CPU->Y = CPU_Read(CPU, Address); CPU_UpdateFlagsNZ(CPU, CPU->Y); }

// Shift Memory One Bit Right Memory
inline void LSR(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
    CPU->SR |= Value & STATUS_CARRY;
    Value >>= 1;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU_Write(CPU, Address, Value);
}

// Shift Accumulator One Bit Right Memory
inline void LSRA(cpu *CPU, u16 Address)
{
    u8 Value = CPU->AC;
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
    CPU->SR |= Value & STATUS_CARRY;
    Value >>= 1;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU->AC = Value;
}

// No Operation
inline void NOP(cpu *CPU, u16 Address)
{
    return;
}

// Not Implemented Operation
inline void _OP(cpu *CPU, u16 Address)
{
	int i = 0;
    return;
}

// OR Accumulator with Value
inline void ORA(cpu *CPU, u16 Address) { CPU->AC |= CPU_Read(CPU, Address); CPU_UpdateFlagsNZ(CPU, CPU->AC); }

// Stack Operations
inline void PHA(cpu *CPU, u16 Address) { CPU_Push(CPU, CPU->AC); }
inline void PHP(cpu *CPU, u16 Address) { CPU_Push(CPU, CPU->SR | 0x30); }
inline void PLA(cpu *CPU, u16 Address) { CPU->AC = CPU_Pull(CPU); CPU_UpdateFlagsNZ(CPU, CPU->AC);}
inline void PLP(cpu *CPU, u16 Address)
{
    u8 NewFlags = CPU_Pull(CPU) | STATUS_;
    if ((CPU->SR ^ NewFlags) & STATUS_INTERRUPT)
        CPU->IRQDelayed = CPU->IRQOccurred;
    CPU->SR = NewFlags;
}

// Rotate Memory One Bit Left
inline void ROL(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    u8 Carry = CPU->SR & STATUS_CARRY;
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
    CPU->SR |= Value >> 7;
    Value <<= 1;
    Value |= Carry;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU_Write(CPU, Address, Value);
}

// Rotate Accumulator One Bit Left
inline void ROLA(cpu *CPU, u16 Address)
{
    u8 Value = CPU->AC;
    u8 Carry = CPU->SR & STATUS_CARRY;
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
    CPU->SR |= Value >> 7;
    Value <<= 1;
    Value |= Carry;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU->AC = Value;
}

// Rotate Memory One Bit Right
inline void ROR(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    u8 Carry = (CPU->SR & STATUS_CARRY) << 7;
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
    CPU->SR |= Value & STATUS_CARRY;
    Value >>= 1;
    Value |= Carry;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU_Write(CPU, Address, Value);
}

// Rotate Accumulator One Bit Right
inline void RORA(cpu *CPU, u16 Address)
{
    u8 Value = CPU->AC;
    u8 Carry = (CPU->SR & STATUS_CARRY) << 7;
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
    CPU->SR |= Value & STATUS_CARRY;
    Value >>= 1;
    Value |= Carry;
    if (Value == 0) CPU->SR |= STATUS_ZERO;
    if (Value & 0x80) CPU->SR |= STATUS_NEGATIVE;
    CPU->AC = Value;
}

// Return from Interrupt
inline void RTI(cpu *CPU, u16 Address)
{
    CPU->PC = 0;
    CPU->SR = CPU_Pull(CPU) | STATUS_;
    CPU->PC |= CPU_Pull(CPU) & 0xFF;
    CPU->PC |= CPU_Pull(CPU) << 8;
}

// Return from Subroutine
inline void RTS(cpu *CPU, u16 Address)
{
    CPU->PC = 0;
    CPU->PC |= CPU_Pull(CPU) & 0xFF;
    CPU->PC |= CPU_Pull(CPU) << 8;
    ++CPU->PC;
}

// Subtract Memory from Accumulator with Borrow
inline void SBC(cpu *CPU, u16 Address)
{
    u8 Value = CPU_Read(CPU, Address);
    u16 Intermediate = ((u16)Value + (u16)(CPU->SR & STATUS_CARRY ^ 1));
    Intermediate = (u16)CPU->AC - Intermediate;
    u8 AC = Intermediate & 0xFF;

    CPU->SR &= ~(STATUS_CARRY | STATUS_OVERFLOW | STATUS_ZERO | STATUS_NEGATIVE);
    if (AC == 0) CPU->SR |= STATUS_ZERO;
    if (AC & 0x80) CPU->SR |= STATUS_NEGATIVE;
    if (Intermediate <= 255)
        CPU->SR |= STATUS_CARRY;
    if ((CPU->AC ^ Value) & (CPU->AC ^ AC) & 0x80)
        CPU->SR |= STATUS_OVERFLOW;
    
    CPU->AC = AC;
}

// Set Flags
inline void SEC(cpu *CPU, u16 Address) { CPU->SR |= STATUS_CARRY; }
inline void SED(cpu *CPU, u16 Address) { CPU->SR |= STATUS_DECIMAL; }
inline void SEI(cpu *CPU, u16 Address) { if (!(CPU->SR & STATUS_INTERRUPT)) { CPU->SR |= STATUS_INTERRUPT; CPU->IRQDelayed = CPU->IRQOccurred; } }

// Store Accumulator in Memory
inline void STA(cpu *CPU, u16 Address) { CPU_Write(CPU, Address, CPU->AC); }
inline void STX(cpu *CPU, u16 Address) { CPU_Write(CPU, Address, CPU->X); }
inline void STY(cpu *CPU, u16 Address) { CPU_Write(CPU, Address, CPU->Y); }

// Transfer Operations
inline void TAX(cpu *CPU, u16 Address) { CPU->X = CPU->AC; CPU_UpdateFlagsNZ(CPU, CPU->X); }
inline void TAY(cpu *CPU, u16 Address) { CPU->Y = CPU->AC; CPU_UpdateFlagsNZ(CPU, CPU->Y); }
inline void TSX(cpu *CPU, u16 Address) { CPU->X = CPU->SP; CPU_UpdateFlagsNZ(CPU, CPU->X); }
inline void TXA(cpu *CPU, u16 Address) { CPU->AC = CPU->X; CPU_UpdateFlagsNZ(CPU, CPU->AC); }
inline void TXS(cpu *CPU, u16 Address) { CPU->SP = CPU->X; }
inline void TYA(cpu *CPU, u16 Address) { CPU->AC = CPU->Y; CPU_UpdateFlagsNZ(CPU, CPU->AC); }

inline u16 CPU_Abs(cpu *CPU)  { return CPU_NextWord(CPU); }
inline u16 CPU_AbsX(cpu *CPU) { return (CPU_NextWord(CPU) + CPU->X); }
inline u16 CPU_AbsY(cpu *CPU) { return (CPU_NextWord(CPU) + CPU->Y); }
inline u16 CPU_Imm(cpu *CPU)  { return CPU->PC++; }
inline u16 CPU_Ind(cpu *CPU)  { return CPU_ReadWordWrap(CPU, CPU_NextWord(CPU)); }
inline u16 CPU_XInd(cpu *CPU) { return CPU_ReadWordWrap(CPU, (CPU_NextByte(CPU) + CPU->X) &0xFF); }
inline u16 CPU_IndY(cpu *CPU) { return CPU_ReadWordWrap(CPU, CPU_NextByte(CPU)) + CPU->Y; }
inline u16 CPU_Rel(cpu *CPU)  { return ((i16)CPU_NextSByte(CPU) + CPU->PC); }
inline u16 CPU_Zpg(cpu *CPU)  { return CPU_NextByte(CPU); }
inline u16 CPU_ZpgX(cpu *CPU) { return (CPU_NextByte(CPU) + CPU->X) & 0xFF; }
inline u16 CPU_ZpgY(cpu *CPU) { return (CPU_NextByte(CPU) + CPU->Y) & 0xFF; }

enum cpu_address_mode
{
    CPU_AM_IMPL = 0,
    CPU_AM_ACC  = 1,
    CPU_AM_ABS  = 2,
    CPU_AM_ABSX = 3,
    CPU_AM_ABSY = 4,
    CPU_AM_IMM  = 5,
    CPU_AM_IND  = 6,
    CPU_AM_XIND = 7,
    CPU_AM_INDY = 8,
    CPU_AM_REL  = 9,
    CPU_AM_ZPG  = 10,
    CPU_AM_ZPGX = 11,
    CPU_AM_ZPGY = 12,
};

static u8 OpAdressModes[256] = {
    0, 7, 0, 0,  0, 10, 10, 0, 0, 5, 1, 0, 0, 2, 2, 0,  9, 8, 0, 0,  0, 11, 11, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
    2, 7, 0, 0, 10, 10, 10, 0, 0, 5, 1, 0, 2, 2, 2, 0,  9, 8, 0, 0,  0, 11, 11, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
    0, 7, 0, 0,  0, 10, 10, 0, 0, 5, 1, 0, 2, 2, 2, 0,  9, 8, 0, 0,  0, 11, 11, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
    0, 7, 0, 0,  0, 10, 10, 0, 0, 5, 1, 0, 6, 2, 2, 0,  9, 8, 0, 0,  0, 11, 11, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
    0, 7, 0, 0, 10, 10, 10, 0, 0, 0, 0, 0, 2, 2, 2, 0,  9, 8, 0, 0, 11, 11, 12, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
    5, 7, 5, 0, 10, 10, 10, 0, 0, 5, 0, 0, 2, 2, 2, 0,  9, 8, 0, 0, 11, 11, 12, 0, 0, 4, 0, 0, 3, 3, 4, 0, 
    5, 7, 0, 0, 10, 10, 10, 0, 0, 5, 0, 0, 2, 2, 2, 0,  9, 8, 0, 0,  0, 11, 11, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
    5, 7, 0, 0, 10, 10, 10, 0, 0, 5, 0, 0, 2, 2, 2, 0,  9, 8, 0, 0,  0, 11, 11, 0, 0, 4, 0, 0, 0, 3, 3, 0, 
};

static u8 OpCycles[256] = {
    7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, 
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, 
    6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, 
    6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, 
    0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,  2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0, 
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,  2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0, 
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, 
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, 
};

static u8 OpPageCycles[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 
};

static u8 OpCodes[256] = {
    11, 35,  0,  0,  0, 35,  3,  0, 37, 35, 57,  0,  0, 35,  3,  0, 10, 35,  0,  0,  0, 35,  3,  0, 14, 35,  0,  0,  0, 35,  3,  0,
    29,  2,  0,  0,  7,  2, 40,  0, 39,  2, 58,  0,  7,  2, 40,  0,  8,  2,  0,  0,  0,  2, 40,  0, 45,  2,  0,  0,  0,  2, 40,  0,
    42, 24,  0,  0,  0, 24, 33,  0, 36, 24, 59,  0, 28, 24, 33,  0, 12, 24,  0,  0,  0, 24, 33,  0, 16, 24,  0,  0,  0, 24, 33,  0,
    43,  1,  0,  0,  0,  1, 41,  0, 38,  1, 60,  0, 28,  1, 41,  0, 13,  1,  0,  0,  0,  1, 41,  0, 47,  1,  0,  0,  0,  1, 41,  0,
     0, 48,  0,  0, 50, 48, 49,  0, 23,  0, 54,  0, 50, 48, 49,  0,  4, 48,  0,  0, 50, 48, 49,  0, 56, 48, 55,  0,  0, 48,  0,  0,
    32, 30, 31,  0, 32, 30, 31,  0, 52, 30, 51,  0, 32, 30, 31,  0,  5, 30,  0,  0, 32, 30, 31,  0, 17, 30, 53,  0, 32, 30, 31,  0,
    20, 18,  0,  0, 20, 18, 21,  0, 27, 18, 22,  0, 20, 18, 21,  0,  9, 18,  0,  0,  0, 18, 21,  0, 15, 18,  0,  0,  0, 18, 21,  0,
    19, 44,  0,  0, 19, 44, 25,  0, 26, 44, 34,  0, 19, 44, 25,  0,  6, 44,  0,  0,  0, 44, 25,  0, 46, 44,  0,  0,  0, 44, 25,  0,
};

typedef void op_func (cpu *CPU, u16 Address);
static op_func *Ops[64] = {
    _OP,
    ADC, AND, ASL, BCC, //  4
    BCS, BEQ, BIT, BMI, //  8
    BNE, BPL, BRK, BVC, // 12
    BVS, CLC, CLD, CLI, // 16
    CLV, CMP, CPX, CPY, // 20
    DEC, DEX, DEY, EOR, // 24
    INC, INX, INY, JMP, // 28
    JSR, LDA, LDX, LDY, // 32
    LSR, NOP, ORA, PHA, // 36
    PHP, PLA, PLP, ROL, // 40
    ROR, RTI, RTS, SBC, // 44
    SEC, SED, SEI, STA, // 48
    STX, STY, TAX, TAY, // 52
    TSX, TXA, TXS, TYA, // 56
    ASLA, ROLA, LSRA, RORA, // 60
    0, 0, 0,
};

// Non Maskable Interrupt
inline void NMI(cpu *CPU)
{
    CPU->NMIOccurred = 0;
    CPU_Push(CPU, CPU->PC >> 8);
    CPU_Push(CPU, CPU->PC & 0xFF);
    CPU_Push(CPU, CPU->SR);
    CPU->PC = CPU_ReadWord(CPU, 0xFFFA);
    CPU->SR |= STATUS_INTERRUPT;
    CPU->Busy += 7;
}

// Interrupt
inline void IRQ(cpu *CPU)
{
    if (CPU->IRQDelayed && !(CPU->SR & STATUS_INTERRUPT)) return;
    if (!CPU->IRQDelayed && CPU->SR & STATUS_INTERRUPT) return;
    CPU_Push(CPU, CPU->PC >> 8);
    CPU_Push(CPU, CPU->PC & 0xFF);
    CPU_Push(CPU, CPU->SR);
    CPU->PC = CPU_ReadWord(CPU, 0xFFFE);
    CPU->SR |= STATUS_INTERRUPT;
    CPU->Busy += 7;
}

inline void CPU_Step(cpu *CPU)
{
    ++CPU->Cycles;
    if (CPU->Cycles <= CPU->Busy)
        return;

    if (CPU->NMIOccurred)
        NMI(CPU);
    else if (CPU->IRQOccurred)
        IRQ(CPU);

    CPU->IRQDelayed = 0;

    u8 Op = CPU_NextByte(CPU);
    u16 Address = 0;
    b32 PageCross = false;
    u8 AddressMode = OpAdressModes[Op];
    switch (AddressMode)
    {
        case CPU_AM_IMPL: break;
        case CPU_AM_ACC:  break;
        case CPU_AM_ABS:  Address = CPU_Abs(CPU); break;
        case CPU_AM_ABSX: Address = CPU_AbsX(CPU); PageCross = CPU_PageCross(Address, Address - (i16)CPU->X);break;
        case CPU_AM_ABSY: Address = CPU_AbsY(CPU); PageCross = CPU_PageCross(Address, Address - (i16)CPU->Y); break;
        case CPU_AM_IMM:  Address = CPU_Imm(CPU); break;
        case CPU_AM_IND:  Address = CPU_Ind(CPU); break;
        case CPU_AM_XIND: Address = CPU_XInd(CPU); break;
        case CPU_AM_INDY: Address = CPU_IndY(CPU); PageCross = CPU_PageCross(Address, Address - (i16)CPU->Y); break;
        case CPU_AM_REL:  Address = CPU_Rel(CPU); break;
        case CPU_AM_ZPG:  Address = CPU_Zpg(CPU); break;
        case CPU_AM_ZPGX: Address = CPU_ZpgX(CPU); break;
        case CPU_AM_ZPGY: Address = CPU_ZpgY(CPU); break;
    }
    CPU->Busy += OpCycles[Op];
    if (PageCross) CPU->Busy += OpPageCycles[Op];
    Ops[OpCodes[Op]](CPU, Address);
}

static void
CPU_Free(cpu *CPU)
{
    Api_Free(CPU);
}

static void
CPU_Reset(cpu *CPU)
{
    CPU->SP -= 3;
    CPU->SR |= 0x4;
    CPU->PC = CPU_ReadWord(CPU, 0xFFFC);
    CPU_Write(CPU, 0x4015, 0);
}

static void
CPU_Power(cpu *CPU)
{
    CPU->AC = 0;
    CPU->X = 0;
    CPU->Y = 0;
    CPU->SP = 0; 
    CPU->SR = STATUS_ | STATUS_BREAK;
    CPU_Reset(CPU);
}

static cpu*
CPU_Create(console *Console)
{
    cpu *CPU = (cpu *)Api_Malloc(sizeof(cpu));
    CPU->Console = Console;    
    return CPU;
}
