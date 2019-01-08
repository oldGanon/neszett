
struct cpu_op
{
    u8 InterruptCheck;
    u8 CycleOp[7];
};

struct cpu
{
    console *Console;
    u64 Cycles;

    cpu_op OP;
    u16 Address;
    u8 OPCycle;
    u8 Data;

    u16 DMACycle;
    u16 DMAAddress;
    u8 DMAActive;

    u8 NMIOccurred;
    u8 IRQOccurred;
    u8 Interrupt;

    u16 PC; // Program Counter
    u8 A;  // Accumulator
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

enum cpu_cycle_op
{
    // FETCH
    FETCH_OP = 0,
    FETCH_ZPG,
    FETCH_DATA,
    FETCH_ADDR,
    FETCH_ADDR_X,
    FETCH_ADDR_Y,
    // READ
    READ_DATA,
    READ_NEXT,
    READ_PC,
    READ_ZPG,
    READ_ADDR,
    READ_ADDR_Y,
    READ_STACK,
    READ_WRAP,
    // WRITE
    WRITE_DATA,
    // PULL
    PULL_HI_PC,
    PULL_LO_PC,
    PULL_SR,
    PULL_A,
    // PUSH
    PUSH_HI_PC,
    PUSH_LO_PC,
    PUSH_SR_BRK,
    PUSH_SR_IRQ,
    PUSH_SR,
    PUSH_A,
    // JSR
    JSR_BEGIN,
    JSR_HI_PC,
    JSR_LO_PC,
    JSR_END,
    // ADDR
    ADDR_ADD_X,
    ADDR_ADD_Y,
    ADDR_ADD_X_C,
    ADDR_ADD_Y_C,
    ADDR_CARRY,
    // REG
    ADC,
    AND,
    BIT,
    CMP,
    CPX,
    CPY,
    EOR,
    LDA,
    LDX,
    LDY,
    ORA,
    SBC,
    STA,
    STX,
    STY,
    DEX,
    DEY,
    INX,
    INY,
    // RMW
    ASL,
    ASL_ACC,
    DEC,
    INC,
    LSR,
    LSR_ACC,
    ROL,
    ROL_ACC,
    ROR,
    ROR_ACC,
    // BRANCH
    BCC,
    BCS,
    BEQ,
    BMI,
    BNE,
    BPL,
    BVC,
    BVS,
    BRANCH_LO_PC,
    BRANCH_HI_PC,
    // FLAGS
    CLC,
    CLD,
    CLI,
    CLV,
    SEC,
    SED,
    SEI,
    // TRANSFER
    TAX,
    TAY,
    TSX,
    TXA,
    TXS,
    TYA,
};

#define REG_OP_imm(OP)  { 0, OP,         FETCH_OP }
#define REG_OP_zpg(OP)  { 1, FETCH_ZPG,  OP,           FETCH_OP }
#define REG_OP_zpgX(OP) { 2, FETCH_ZPG,  ADDR_ADD_X,   OP,          FETCH_OP }
#define REG_OP_zpgY(OP) { 2, FETCH_ZPG,  ADDR_ADD_Y,   OP,          FETCH_OP }
#define REG_OP_abs(OP)  { 2, FETCH_DATA, FETCH_ADDR,   OP,          FETCH_OP }
#define REG_OP_absX(OP) { 3, FETCH_DATA, FETCH_ADDR_X, ADDR_CARRY,  OP,         FETCH_OP } //
#define REG_OP_absY(OP) { 3, FETCH_DATA, FETCH_ADDR_Y, ADDR_CARRY,  OP,         FETCH_OP } //
#define REG_OP_indX(OP) { 4, FETCH_ZPG,  ADDR_ADD_X,   READ_WRAP,   READ_ADDR,  OP,      FETCH_OP }
#define REG_OP_indY(OP) { 4, FETCH_ZPG,  READ_WRAP,    READ_ADDR_Y, ADDR_CARRY, OP,      FETCH_OP } //

#define ADC_imm  REG_OP_imm (ADC)
#define ADC_zpg  REG_OP_zpg (ADC)
#define ADC_zpgX REG_OP_zpgX(ADC)
#define ADC_abs  REG_OP_abs (ADC)
#define ADC_absX REG_OP_absX(ADC)
#define ADC_absY REG_OP_absY(ADC)
#define ADC_indX REG_OP_indX(ADC)
#define ADC_indY REG_OP_indY(ADC)

#define AND_imm  REG_OP_imm (AND)
#define AND_zpg  REG_OP_zpg (AND)
#define AND_zpgX REG_OP_zpgX(AND)
#define AND_abs  REG_OP_abs (AND)
#define AND_absX REG_OP_absX(AND)
#define AND_absY REG_OP_absY(AND)
#define AND_indX REG_OP_indX(AND)
#define AND_indY REG_OP_indY(AND)

#define BIT_imm  { }
#define BIT_zpg  REG_OP_zpg (BIT)
#define BIT_zpgX { }
#define BIT_abs  REG_OP_abs (BIT)
#define BIT_absX { }
#define BIT_absY { }
#define BIT_indX { }
#define BIT_indY { }

#define CMP_imm  REG_OP_imm (CMP)
#define CMP_zpg  REG_OP_zpg (CMP)
#define CMP_zpgX REG_OP_zpgX(CMP)
#define CMP_abs  REG_OP_abs (CMP)
#define CMP_absX REG_OP_absX(CMP)
#define CMP_absY REG_OP_absY(CMP)
#define CMP_indX REG_OP_indX(CMP)
#define CMP_indY REG_OP_indY(CMP)

#define CPX_imm  REG_OP_imm(CPX)
#define CPX_zpg  REG_OP_zpg(CPX)
#define CPX_zpgX { }
#define CPX_abs  REG_OP_abs(CPX)
#define CPX_absX { }
#define CPX_absY { }
#define CPX_indX { }
#define CPX_indY { }

#define CPY_imm  REG_OP_imm(CPY)
#define CPY_zpg  REG_OP_zpg(CPY)
#define CPY_zpgX { }
#define CPY_abs  REG_OP_abs(CPY)
#define CPY_absX { }
#define CPY_absY { }
#define CPY_indX { }
#define CPY_indY { }

#define EOR_imm  REG_OP_imm (EOR)
#define EOR_zpg  REG_OP_zpg (EOR)
#define EOR_zpgX REG_OP_zpgX(EOR)
#define EOR_abs  REG_OP_abs (EOR)
#define EOR_absX REG_OP_absX(EOR)
#define EOR_absY REG_OP_absY(EOR)
#define EOR_indX REG_OP_indX(EOR)
#define EOR_indY REG_OP_indY(EOR)

#define LDA_imm  REG_OP_imm (LDA)
#define LDA_zpg  REG_OP_zpg (LDA)
#define LDA_zpgX REG_OP_zpgX(LDA)
#define LDA_abs  REG_OP_abs (LDA)
#define LDA_absX REG_OP_absX(LDA)
#define LDA_absY REG_OP_absY(LDA)
#define LDA_indX REG_OP_indX(LDA)
#define LDA_indY REG_OP_indY(LDA)

#define LDX_imm  REG_OP_imm (LDX)
#define LDX_zpg  REG_OP_zpg (LDX)
#define LDX_zpgY REG_OP_zpgY(LDX)
#define LDX_abs  REG_OP_abs (LDX)
#define LDX_absX { }
#define LDX_absY REG_OP_absY(LDX)
#define LDX_indX { }
#define LDX_indY { }

#define LDY_imm  REG_OP_imm (LDY)
#define LDY_zpg  REG_OP_zpg (LDY)
#define LDY_zpgX REG_OP_zpgX(LDY)
#define LDY_abs  REG_OP_abs (LDY)
#define LDY_absX REG_OP_absX(LDY)
#define LDY_absY { }
#define LDY_indX { }
#define LDY_indY { }

#define ORA_imm  REG_OP_imm (ORA)
#define ORA_zpg  REG_OP_zpg (ORA)
#define ORA_zpgX REG_OP_zpgX(ORA)
#define ORA_abs  REG_OP_abs (ORA)
#define ORA_absX REG_OP_absX(ORA)
#define ORA_absY REG_OP_absY(ORA)
#define ORA_indX REG_OP_indX(ORA)
#define ORA_indY REG_OP_indY(ORA)

#define SBC_imm  REG_OP_imm (SBC)
#define SBC_zpg  REG_OP_zpg (SBC)
#define SBC_zpgX REG_OP_zpgX(SBC)
#define SBC_abs  REG_OP_abs (SBC)
#define SBC_absX REG_OP_absX(SBC)
#define SBC_absY REG_OP_absY(SBC)
#define SBC_indX REG_OP_indX(SBC)
#define SBC_indY REG_OP_indY(SBC)

#define STORE_OP_zpg(OP)  { 1, FETCH_ZPG,  OP,         FETCH_OP }
#define STORE_OP_zpgX(OP) { 2, FETCH_ZPG,  ADDR_ADD_X, OP,           FETCH_OP }
#define STORE_OP_zpgY(OP) { 2, FETCH_ZPG,  ADDR_ADD_Y, OP,           FETCH_OP }
#define STORE_OP_abs(OP)  { 2, FETCH_DATA, FETCH_ADDR, OP,           FETCH_OP }
#define STORE_OP_absX(OP) { 3, FETCH_DATA, FETCH_ADDR, ADDR_ADD_X_C, OP,           FETCH_OP } //
#define STORE_OP_absY(OP) { 3, FETCH_DATA, FETCH_ADDR, ADDR_ADD_Y_C, OP,           FETCH_OP } //
#define STORE_OP_indX(OP) { 4, FETCH_ZPG,  ADDR_ADD_X, READ_WRAP,    READ_ADDR,    OP,      FETCH_OP }
#define STORE_OP_indY(OP) { 4, FETCH_ZPG,  READ_WRAP,  READ_ADDR,    ADDR_ADD_Y_C, OP,      FETCH_OP } //

#define STA_zpg  STORE_OP_zpg (STA)
#define STA_zpgX STORE_OP_zpgX(STA)
#define STA_abs  STORE_OP_abs (STA)
#define STA_absX STORE_OP_absX(STA)
#define STA_absY STORE_OP_absY(STA)
#define STA_indX STORE_OP_indX(STA)
#define STA_indY STORE_OP_indY(STA)

#define STX_zpg  STORE_OP_zpg (STX)
#define STX_zpgY STORE_OP_zpgY(STX)
#define STX_abs  STORE_OP_abs (STX)
#define STX_absX { }
#define STX_absY { }
#define STX_indX { }
#define STX_indY { }

#define STY_zpg  STORE_OP_zpg (STY)
#define STY_zpgX STORE_OP_zpgX(STY)
#define STY_abs  STORE_OP_abs (STY)
#define STY_absX { }
#define STY_absY { }
#define STY_indX { }
#define STY_indY { }

#define DEX_imp { 0, DEX, FETCH_OP }
#define DEY_imp { 0, DEY, FETCH_OP }

#define INX_imp { 0, INX, FETCH_OP }
#define INY_imp { 0, INY, FETCH_OP }

#define RMW_OP_acc(OP)  { 0, OP##_ACC,   FETCH_OP }
#define RMW_OP_zpg(OP)  { 3, FETCH_ZPG,  READ_DATA,  OP,           WRITE_DATA, FETCH_OP }
#define RMW_OP_zpgX(OP) { 4, FETCH_ZPG,  ADDR_ADD_X, READ_DATA,    OP,         WRITE_DATA, FETCH_OP }
#define RMW_OP_abs(OP)  { 4, FETCH_DATA, FETCH_ADDR, READ_DATA,    OP,         WRITE_DATA, FETCH_OP }
#define RMW_OP_absX(OP) { 5, FETCH_DATA, FETCH_ADDR, ADDR_ADD_X_C, READ_DATA,  OP,         WRITE_DATA, FETCH_OP }

#define ASL_acc  RMW_OP_acc (ASL)
#define ASL_zpg  RMW_OP_zpg (ASL)
#define ASL_zpgX RMW_OP_zpgX(ASL)
#define ASL_abs  RMW_OP_abs (ASL)
#define ASL_absX RMW_OP_absX(ASL)

#define DEC_acc  { }
#define DEC_zpg  RMW_OP_zpg (DEC)
#define DEC_zpgX RMW_OP_zpgX(DEC)
#define DEC_abs  RMW_OP_abs (DEC)
#define DEC_absX RMW_OP_absX(DEC)

#define INC_acc  { }
#define INC_zpg  RMW_OP_zpg (INC)
#define INC_zpgX RMW_OP_zpgX(INC)
#define INC_abs  RMW_OP_abs (INC)
#define INC_absX RMW_OP_absX(INC)

#define LSR_acc  RMW_OP_acc (LSR)
#define LSR_zpg  RMW_OP_zpg (LSR)
#define LSR_zpgX RMW_OP_zpgX(LSR)
#define LSR_abs  RMW_OP_abs (LSR)
#define LSR_absX RMW_OP_absX(LSR)

#define ROL_acc  RMW_OP_acc (ROL)
#define ROL_zpg  RMW_OP_zpg (ROL)
#define ROL_zpgX RMW_OP_zpgX(ROL)
#define ROL_abs  RMW_OP_abs (ROL)
#define ROL_absX RMW_OP_absX(ROL)

#define ROR_acc  RMW_OP_acc (ROR)
#define ROR_zpg  RMW_OP_zpg (ROR)
#define ROR_zpgX RMW_OP_zpgX(ROR)
#define ROR_abs  RMW_OP_abs (ROR)
#define ROR_absX RMW_OP_absX(ROR)

#define BCC_rel { 2, BCC, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BCS_rel { 2, BCS, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BEQ_rel { 2, BEQ, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BMI_rel { 2, BMI, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BNE_rel { 2, BNE, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BPL_rel { 2, BPL, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BVC_rel { 2, BVC, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }
#define BVS_rel { 2, BVS, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_OP }

#define CLC_imp { 0, CLC, FETCH_OP }
#define CLD_imp { 0, CLD, FETCH_OP }
#define CLI_imp { 0, CLI, FETCH_OP }
#define CLV_imp { 0, CLV, FETCH_OP }

#define SEC_imp { 0, SEC, FETCH_OP }
#define SED_imp { 0, SED, FETCH_OP }
#define SEI_imp { 0, SEI, FETCH_OP }

#define BRK_imp { 0xFF, FETCH_DATA, PUSH_HI_PC, PUSH_LO_PC, PUSH_SR_BRK, READ_NEXT, READ_PC, FETCH_OP }
#define IRQ_imp { 0xFF, READ_DATA,  PUSH_HI_PC, PUSH_LO_PC, PUSH_SR_IRQ, READ_NEXT, READ_PC, FETCH_OP }

#define JMP_abs { 1, READ_NEXT,  READ_PC,   FETCH_OP }
#define JMP_ind { 3, READ_NEXT,  READ_ADDR, READ_WRAP, READ_PC,   FETCH_OP }
#define JSR_abs { 4, FETCH_DATA, JSR_BEGIN, JSR_HI_PC, JSR_LO_PC, JSR_END, FETCH_OP }

#define NOP_imp { 0, READ_DATA, FETCH_OP }

#define PHA_imp { 1, READ_DATA, PUSH_A,  FETCH_OP }
#define PHP_imp { 1, READ_DATA, PUSH_SR, FETCH_OP }

#define PLA_imp { 2, READ_DATA, READ_STACK, PULL_A,  FETCH_OP }
#define PLP_imp { 2, READ_DATA, READ_STACK, PULL_SR, FETCH_OP }

#define RTI_imp { 4, READ_DATA, READ_STACK, PULL_SR,    PULL_LO_PC, PULL_HI_PC, FETCH_OP }
#define RTS_imp { 4, READ_DATA, READ_STACK, PULL_LO_PC, PULL_HI_PC, FETCH_DATA, FETCH_OP }

#define TAX_imp { 0, TAX, FETCH_OP }
#define TAY_imp { 0, TAY, FETCH_OP }
#define TSX_imp { 0, TSX, FETCH_OP }
#define TXA_imp { 0, TXA, FETCH_OP }
#define TXS_imp { 0, TXS, FETCH_OP }
#define TYA_imp { 0, TYA, FETCH_OP }

const cpu_op OP_Table[256] =
{
    BRK_imp, ORA_indX, {},      {}, {},       ORA_zpg,  ASL_zpg,  {}, PHP_imp, ORA_imm,  ASL_acc, {}, {},       ORA_abs,  ASL_abs,  {},
    BPL_rel, ORA_indY, {},      {}, {},       ORA_zpgX, ASL_zpgX, {}, CLC_imp, ORA_absY, {},      {}, {},       ORA_absX, ASL_absX, {},
    JSR_abs, AND_indX, {},      {}, BIT_zpg,  AND_zpg,  ROL_zpg,  {}, PLP_imp, AND_imm,  ROL_acc, {}, BIT_abs,  AND_abs,  ROL_abs,  {},
    BMI_rel, AND_indY, {},      {}, {},       AND_zpgX, ROL_zpgX, {}, SEC_imp, AND_absY, {},      {}, {},       AND_absX, ROL_absX, {},
    RTI_imp, EOR_indX, {},      {}, {},       EOR_zpg,  LSR_zpg,  {}, PHA_imp, EOR_imm,  LSR_acc, {}, JMP_abs,  EOR_abs,  LSR_abs,  {},
    BVC_rel, EOR_indY, {},      {}, {},       EOR_zpgX, LSR_zpgX, {}, CLI_imp, EOR_absY, {},      {}, {},       EOR_absX, LSR_absX, {},
    RTS_imp, ADC_indX, {},      {}, {},       ADC_zpg,  ROR_zpg,  {}, PLA_imp, ADC_imm,  ROR_acc, {}, JMP_ind,  ADC_abs,  ROR_abs,  {},
    BVS_rel, ADC_indY, {},      {}, {},       ADC_zpgX, ROR_zpgX, {}, SEI_imp, ADC_absY, {},      {}, {},       ADC_absX, ROR_absX, {},
    {},      STA_indX, {},      {}, STY_zpg,  STA_zpg,  STX_zpg,  {}, DEY_imp, {},       TXA_imp, {}, STY_abs,  STA_abs,  STX_abs,  {},
    BCC_rel, STA_indY, {},      {}, STY_zpgX, STA_zpgX, STX_zpgY, {}, TYA_imp, STA_absY, TXS_imp, {}, {},       STA_absX, {},       {},
    LDY_imm, LDA_indX, LDX_imm, {}, LDY_zpg,  LDA_zpg,  LDX_zpg,  {}, TAY_imp, LDA_imm,  TAX_imp, {}, LDY_abs,  LDA_abs,  LDX_abs,  {},
    BCS_rel, LDA_indY, {},      {}, LDY_zpgX, LDA_zpgX, LDX_zpgY, {}, CLV_imp, LDA_absY, TSX_imp, {}, LDY_absX, LDA_absX, LDX_absY, {},
    CPY_imm, CMP_indX, {},      {}, CPY_zpg,  CMP_zpg,  DEC_zpg,  {}, INY_imp, CMP_imm,  DEX_imp, {}, CPY_abs,  CMP_abs,  DEC_abs,  {},
    BNE_rel, CMP_indY, {},      {}, {},       CMP_zpgX, DEC_zpgX, {}, CLD_imp, CMP_absY, {},      {}, {},       CMP_absX, DEC_absX, {},
    CPX_imm, SBC_indX, {},      {}, CPX_zpg,  SBC_zpg,  INC_zpg,  {}, INX_imp, SBC_imm,  NOP_imp, {}, CPX_abs,  SBC_abs,  INC_abs,  {},
    BEQ_rel, SBC_indY, {},      {}, {},       SBC_zpgX, INC_zpgX, {}, SED_imp, SBC_absY, {},      {}, {},       SBC_absX, INC_absX, {},
};

inline void
CPU_DMA(cpu *CPU, u8 Value)
{
    CPU->DMAActive = true;
    CPU->DMACycle = (CPU->Cycles & 1) ^ 1;
    CPU->DMAAddress = (u16)(Value & 0x7) << 8;
}

inline u8
CPU_R(cpu *CPU, u16 Address)
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
CPU_W(cpu *CPU, u16 Address, u8 Value)
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

inline void
CPU_DMACopy(cpu *CPU)
{
    u16 Cycle = CPU->DMACycle++;
    if (Cycle < 2)
    {
        CPU_R(CPU, CPU->DMAAddress);
        return;
    }
    
    if (Cycle & 1)
        CPU_W(CPU, 0x2004, CPU->Data);
    else
        CPU->Data = CPU_R(CPU, CPU->DMAAddress++);

    if (Cycle >= 513)
        CPU->DMAActive = false;
}

// FLAGS

inline void
CPU_ClearNVZC(cpu *CPU)
{
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_OVERFLOW | STATUS_ZERO | STATUS_CARRY);
}

inline void
CPU_ClearNZC(cpu *CPU)
{
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO | STATUS_CARRY);
}

inline void
CPU_ClearNVZ(cpu *CPU)
{
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_OVERFLOW | STATUS_ZERO);
}

inline void
CPU_ClearNZ(cpu *CPU)
{
    CPU->SR &= ~(STATUS_NEGATIVE | STATUS_ZERO);
}

inline void
CPU_ClearN(cpu *CPU)
{
    CPU->SR &= ~STATUS_NEGATIVE;
}

inline void
CPU_ClearV(cpu *CPU)
{
    CPU->SR &= ~STATUS_OVERFLOW;
}

inline void
CPU_ClearD(cpu *CPU)
{
    CPU->SR &= ~STATUS_DECIMAL;
}

inline void
CPU_ClearI(cpu *CPU)
{
    CPU->SR &= ~STATUS_INTERRUPT;
}

inline void
CPU_ClearZ(cpu *CPU)
{
    CPU->SR &= ~STATUS_ZERO;
}

inline void
CPU_ClearC(cpu *CPU)
{
    CPU->SR &= ~STATUS_CARRY;
}

inline void
CPU_SetN(cpu *CPU)
{
    CPU->SR |= STATUS_NEGATIVE;
}

inline void
CPU_SetV(cpu *CPU)
{
    CPU->SR |= STATUS_OVERFLOW;
}

inline void
CPU_SetD(cpu *CPU)
{
    CPU->SR |= STATUS_DECIMAL;
}

inline void
CPU_SetI(cpu *CPU)
{
    CPU->SR |= STATUS_INTERRUPT;
}

inline void
CPU_SetZ(cpu *CPU)
{
    CPU->SR |= STATUS_ZERO;
}

inline void
CPU_SetC(cpu *CPU)
{
    CPU->SR |= STATUS_CARRY;
}

// READS FROM PC LOCATION

inline void
CPU_Fetch(cpu *CPU)
{
    CPU->Data = CPU_R(CPU, CPU->PC++);
}

inline void
CPU_FetchZpg(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->Address = CPU->Data;
}

inline void
CPU_FetchAddr(cpu *CPU)
{
    u8 Lo = CPU->Data;
    CPU_Fetch(CPU);
    CPU->Address = Lo | (CPU->Data << 8);
}

inline void
CPU_FetchAddrX(cpu *CPU)
{
    u8 Lo = CPU->Data;
    CPU_Fetch(CPU);
    CPU->Address = (Lo | (CPU->Data << 8));
    u16 NewAddress = CPU->Address + CPU->X;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0xFF);
    if (CPU->Address == NewAddress)
        ++CPU->OPCycle;
}

inline void
CPU_FetchAddrY(cpu *CPU)
{
    u8 Lo = CPU->Data;
    CPU_Fetch(CPU);
    CPU->Address = (Lo | (CPU->Data << 8));
    u16 NewAddress = CPU->Address + CPU->Y;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0xFF);
    if (CPU->Address == NewAddress)
        ++CPU->OPCycle;
}

// READS FROM ADDRESS BUS LOCATION

inline void
CPU_Read(cpu *CPU)
{
    CPU->Data = CPU_R(CPU, CPU->Address);
}

inline void
CPU_ReadNext(cpu *CPU)
{
    CPU_Read(CPU);
    ++CPU->Address;
}

inline void
CPU_ReadWrap(cpu *CPU)
{
    CPU_Read(CPU);
    u8 Lo = ((CPU->Address + 1) & 0xFF);
    CPU->Address =  Lo | (CPU->Address & 0xFF00);
}

inline void
CPU_ReadPC(cpu *CPU)
{
    u8 Lo = CPU->Data;
    CPU_ReadNext(CPU);
    CPU->PC = Lo | (CPU->Data << 8);
}

inline void
CPU_ReadZpg(cpu *CPU)
{
    CPU_ReadNext(CPU);
    CPU->Address = CPU->Data;
}

inline void
CPU_ReadAddr(cpu *CPU)
{
    u8 Lo = CPU->Data;
    CPU_ReadNext(CPU);
    CPU->Address = Lo | (CPU->Data << 8);
}

inline void
CPU_ReadAddrY(cpu *CPU)
{
    u8 Lo = CPU->Data;
    CPU_ReadNext(CPU);
    CPU->Address = (Lo | (CPU->Data << 8));
    u16 NewAddress = CPU->Address + CPU->Y;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0xFF);
    if (CPU->Address == NewAddress)
        ++CPU->OPCycle;
}

inline void
CPU_ReadStack(cpu *CPU)
{
    CPU->Address = CPU->SP | 0x100;
    CPU_Read(CPU);
}

// WRITE

inline void
CPU_Write(cpu *CPU)
{
    CPU_W(CPU, CPU->Address, CPU->Data);
}

// PULL

inline void
CPU_Pull(cpu *CPU)
{
    CPU->Address = ++CPU->SP | 0x100;
    CPU_Read(CPU);
}

inline void
CPU_PullLoPC(cpu *CPU)
{
    CPU_Pull(CPU);
    CPU->PC = (CPU->PC & 0xFF00) | CPU->Data;
}

inline void
CPU_PullHiPC(cpu *CPU)
{
    CPU_Pull(CPU);
    CPU->PC = (CPU->PC & 0xFF) | (CPU->Data << 8);
}

inline void
CPU_PullSR(cpu *CPU)
{
    CPU_Pull(CPU);
    CPU->SR = CPU->Data & ~(STATUS_ | STATUS_BREAK);
}

inline void
CPU_PullA(cpu *CPU)
{
    CPU_Pull(CPU);
    CPU->A = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

// PUSH

inline void
CPU_Push(cpu *CPU)
{
    CPU->Address = CPU->SP-- | 0x100;
    CPU_Write(CPU);
}

inline void
CPU_PushLoPC(cpu *CPU)
{
    CPU->Data = CPU->PC & 0xFF;
    CPU_Push(CPU);
}

inline void
CPU_PushHiPC(cpu *CPU)
{
    CPU->Data = CPU->PC >> 8;
    CPU_Push(CPU);
}

inline void
CPU_PushSRBRK(cpu *CPU)
{
    CPU->Data = CPU->SR | STATUS_ | STATUS_BREAK;
    CPU_Push(CPU);
    CPU->SR |= STATUS_INTERRUPT;
    if (CPU->NMIOccurred)
    {
        CPU->NMIOccurred = 0;
        CPU->Address = 0xFFFA;
    }
    else CPU->Address = 0xFFFE;
    CPU->Interrupt = false;
}

inline void
CPU_PushSRIRQ(cpu *CPU)
{
    CPU->Data = CPU->SR | STATUS_;
    CPU_Push(CPU);
    CPU->SR |= STATUS_INTERRUPT;
    if (CPU->NMIOccurred)
    {
        CPU->NMIOccurred = 0;
        CPU->Address = 0xFFFA;
    }
    else CPU->Address = 0xFFFE;
    CPU->Interrupt = false;
}

inline void
CPU_PushSR(cpu *CPU)
{
    CPU->Data = CPU->SR | STATUS_ | STATUS_BREAK;
    CPU_Push(CPU);
}

inline void
CPU_PushA(cpu *CPU)
{
    CPU->Data = CPU->A;    
    CPU_Push(CPU);
}

// JSR

inline void
CPU_JSRBegin(cpu *CPU)
{
    CPU->Address = CPU->SP | 0x100;
    CPU->SP = CPU->Data;
    CPU_Read(CPU);
}

inline void
CPU_JSRLoPC(cpu *CPU)
{
    CPU->Data = CPU->PC & 0xFF;
    CPU_Write(CPU);
    --CPU->Address;
}

inline void
CPU_JSRHiPC(cpu *CPU)
{
    CPU->Data = CPU->PC >> 8;
    CPU_Write(CPU);
    --CPU->Address;
}

inline void
CPU_JSREnd(cpu *CPU)
{
    u8 Lo = CPU->SP;
    CPU->SP = CPU->Address & 0xFF;
    CPU_Fetch(CPU);
    CPU->PC = Lo | (CPU->Data << 8);
}

// ADDRESS

inline void
CPU_AddrAddX(cpu *CPU)
{
    u8 NewAddress = CPU->Data + CPU->X;
    CPU_Read(CPU);
    CPU->Address = NewAddress;
}

inline void
CPU_AddrAddY(cpu *CPU)
{
    u8 NewAddress = CPU->Data + CPU->Y;
    CPU_Read(CPU);
    CPU->Address = NewAddress;
}

inline void
CPU_AddrAddXC(cpu *CPU)
{
    u16 NewAddress = CPU->Address + CPU->X;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0x00FF);
    CPU_Read(CPU);
    CPU->Address = NewAddress;
}

inline void
CPU_AddrAddYC(cpu *CPU)
{
    u16 NewAddress = CPU->Address + CPU->Y;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0x00FF);
    CPU_Read(CPU);
    CPU->Address = NewAddress;
}

inline void
CPU_AddrCarry(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Address += 0x100;
}

// REG OPS

inline void
CPU_ADC(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    u16 I = (u16)CPU->Data + (u16)(CPU->SR & STATUS_CARRY);
    I = (u16)CPU->A + I;
    u8 A = I & 0xFF;

    CPU_ClearNVZC(CPU);
    if (A == 0) CPU_SetZ(CPU);
    if (A & 0x80) CPU_SetN(CPU);
    if (I > 255) CPU_SetC(CPU);
    if ((CPU->A ^ A) & (CPU->Data ^ A) & 0x80) CPU_SetV(CPU);

    CPU->A = A;
}

inline void
CPU_AND(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU->A &= CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_BIT(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU_ClearNVZ(CPU);
    CPU->SR |= CPU->Data & (STATUS_NEGATIVE | STATUS_OVERFLOW);
    if ((CPU->A & CPU->Data) == 0) CPU_SetZ(CPU);
}

inline void
CPU_CMP(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU_ClearNZC(CPU);
    if (CPU->A >= CPU->Data) CPU_SetC(CPU);
    if (CPU->A == CPU->Data) CPU_SetZ(CPU);
    if ((u8)(CPU->A - CPU->Data) & 0x80) CPU_SetN(CPU);
}

inline void
CPU_CPX(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU_ClearNZC(CPU);
    if (CPU->X >= CPU->Data) CPU_SetC(CPU);
    if (CPU->X == CPU->Data) CPU_SetZ(CPU);
    if ((u8)(CPU->X - CPU->Data) & 0x80) CPU_SetN(CPU);
}

inline void
CPU_CPY(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU_ClearNZC(CPU);
    if (CPU->Y >= CPU->Data) CPU_SetC(CPU);
    if (CPU->Y == CPU->Data) CPU_SetZ(CPU);
    if ((u8)(CPU->Y - CPU->Data) & 0x80) CPU_SetN(CPU);
}

inline void
CPU_EOR(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU->A ^= CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_LDA(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU->A = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}
inline void
CPU_LDX(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU->X = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}
inline void
CPU_LDY(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU->Y = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->Y == 0) CPU_SetZ(CPU);
    if (CPU->Y & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ORA(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    CPU->A |= CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_SBC(cpu *CPU)
{
    CPU_Read(CPU);
    if (CPU->OPCycle == 1) ++CPU->PC;
    u16 I = ((u16)CPU->Data + (u16)(CPU->SR & STATUS_CARRY ^ 1));
    I = (u16)CPU->A - I;
    u8 A = I & 0xFF;

    CPU_ClearNVZC(CPU);
    if (A == 0) CPU_SetZ(CPU);
    if (A & 0x80) CPU_SetN(CPU);
    if (I <= 255) CPU_SetC(CPU);
    if ((CPU->A ^ CPU->Data) & (CPU->A ^ A) & 0x80) CPU_SetV(CPU);
    
    CPU->A = A;
}

inline void
CPU_STA(cpu *CPU)
{
    CPU->Data = CPU->A;
    CPU_Write(CPU);
}

inline void
CPU_STX(cpu *CPU)
{
    CPU->Data = CPU->X;
    CPU_Write(CPU);
}

inline void
CPU_STY(cpu *CPU)
{
    CPU->Data = CPU->Y;
    CPU_Write(CPU);
}

inline void
CPU_DEX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZ(CPU);
    --CPU->X;
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}

inline void
CPU_DEY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZ(CPU);
    --CPU->Y;
    if (CPU->Y == 0) CPU_SetZ(CPU);
    if (CPU->Y & 0x80) CPU_SetN(CPU);
}

inline void
CPU_INX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZ(CPU);
    ++CPU->X;
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}

inline void
CPU_INY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZ(CPU);
    ++CPU->Y;
    if (CPU->Y == 0) CPU_SetZ(CPU);
    if (CPU->Y & 0x80) CPU_SetN(CPU);
}

// RMW OPS

inline void
CPU_ASL(cpu *CPU)
{
    CPU_Write(CPU);
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->Data >> 7;
    CPU->Data <<= 1;
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ASL_ACC(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->A >> 7;
    CPU->A <<= 1;
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_DEC(cpu *CPU)
{
    CPU_Write(CPU);
    CPU_ClearNZ(CPU);
    --CPU->Data;
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_INC(cpu *CPU)
{
    CPU_Write(CPU);
    CPU_ClearNZ(CPU);
    ++CPU->Data;
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_LSR(cpu *CPU)
{
    CPU_Write(CPU);
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->Data & STATUS_CARRY;
    CPU->Data >>= 1;
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_LSR_ACC(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->A & STATUS_CARRY;
    CPU->A >>= 1;
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ROL(cpu *CPU)
{
    CPU_Write(CPU);
    u8 Carry = CPU->SR & STATUS_CARRY;
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->Data >> 7;
    CPU->Data <<= 1;
    CPU->Data |= Carry;
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ROL_ACC(cpu *CPU)
{
    CPU_Read(CPU);
    u8 Carry = CPU->SR & STATUS_CARRY;
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->A >> 7;
    CPU->A <<= 1;
    CPU->A |= Carry;
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ROR(cpu *CPU)
{
    CPU_Write(CPU);
    u8 Carry = (CPU->SR & STATUS_CARRY) << 7;
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->Data & STATUS_CARRY;
    CPU->Data >>= 1;
    CPU->Data |= Carry;
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ROR_ACC(cpu *CPU)
{
    CPU_Read(CPU);
    u8 Carry = (CPU->SR & STATUS_CARRY) << 7;
    CPU_ClearNZC(CPU);
    CPU->SR |= CPU->A & STATUS_CARRY;
    CPU->A >>= 1;
    CPU->A |= Carry;
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

//

inline void
CPU_Branch(cpu *CPU, b32 Condition)
{
    CPU_Fetch(CPU);
    if (Condition)
    {
        CPU->OPCycle = 3;
        CPU->Interrupt = CPU->NMIOccurred || (CPU->IRQOccurred && !(CPU->SR & STATUS_INTERRUPT));
    }
}

inline void CPU_BCC(cpu *CPU) { CPU_Branch(CPU,   CPU->SR & STATUS_CARRY); }
inline void CPU_BCS(cpu *CPU) { CPU_Branch(CPU, !(CPU->SR & STATUS_CARRY)); }
inline void CPU_BEQ(cpu *CPU) { CPU_Branch(CPU, !(CPU->SR & STATUS_ZERO)); }
inline void CPU_BMI(cpu *CPU) { CPU_Branch(CPU, !(CPU->SR & STATUS_NEGATIVE)); }
inline void CPU_BNE(cpu *CPU) { CPU_Branch(CPU,   CPU->SR & STATUS_ZERO); }
inline void CPU_BPL(cpu *CPU) { CPU_Branch(CPU,   CPU->SR & STATUS_NEGATIVE); }
inline void CPU_BVC(cpu *CPU) { CPU_Branch(CPU,   CPU->SR & STATUS_OVERFLOW); }
inline void CPU_BVS(cpu *CPU) { CPU_Branch(CPU, !(CPU->SR & STATUS_OVERFLOW)); }

inline void
CPU_BranchLo(cpu *CPU)
{
    CPU_Read(CPU);
    u16 Old = CPU->PC;
    CPU->PC += (i8)CPU->Data;
    if ((CPU->PC & 0xFF00) == (Old & 0xFF00))
        CPU->OPCycle = 3;
}

inline void
CPU_BranchHi(cpu *CPU)
{
    CPU_Read(CPU);
    // Carry is already done last cycle
}

inline void
CPU_CLC(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearC(CPU);
}

inline void
CPU_CLD(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearD(CPU);
}

inline void
CPU_CLI(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearI(CPU);
}

inline void
CPU_CLV(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearV(CPU);
}

inline void
CPU_SEC(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_SetC(CPU);
}

inline void
CPU_SED(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_SetD(CPU);
}

inline void
CPU_SEI(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_SetI(CPU);
}

inline void
CPU_TAX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->X = CPU->A;
    CPU_ClearNZ(CPU);
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}

inline void
CPU_TAY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Y = CPU->A;
    CPU_ClearNZ(CPU);
    if (CPU->Y == 0) CPU_SetZ(CPU);
    if (CPU->Y & 0x80) CPU_SetN(CPU);
}

inline void
CPU_TSX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->X = CPU->SP;
    CPU_ClearNZ(CPU);
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}

inline void
CPU_TXA(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A = CPU->X;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_TXS(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->SP = CPU->X;
}

inline void
CPU_TYA(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A = CPU->Y;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_FetchOP(cpu *CPU)
{    
    CPU->Address = CPU->PC;
    if (CPU->Interrupt)
    {
        CPU_Read(CPU);
        CPU->OP = IRQ_imp;
    }
    else
    {
        CPU_Fetch(CPU);
        CPU->OP = OP_Table[CPU->Data];
    }
    CPU->OPCycle = 0;
    CPU->Address = CPU->PC;
}

static void
CPU_Step(cpu *CPU)
{
    ++CPU->Cycles;

    if (CPU->DMAActive)
    {
        CPU_DMACopy(CPU);
        return;
    }

    if (CPU->OP.InterruptCheck == CPU->OPCycle)
        CPU->Interrupt = CPU->NMIOccurred || (CPU->IRQOccurred && !(CPU->SR & STATUS_INTERRUPT));

    switch (CPU->OP.CycleOp[CPU->OPCycle++])
    {
        // FETCH
        case FETCH_OP:      CPU_FetchOP(CPU); break;
        case FETCH_ZPG:     CPU_FetchZpg(CPU); break;
        case FETCH_DATA:    CPU_Fetch(CPU); break;
        case FETCH_ADDR:    CPU_FetchAddr(CPU); break;
        case FETCH_ADDR_X:  CPU_FetchAddrX(CPU); break;
        case FETCH_ADDR_Y:  CPU_FetchAddrY(CPU); break;
        // READ
        case READ_DATA:   CPU_Read(CPU); break;
        case READ_NEXT:   CPU_ReadNext(CPU); break;
        case READ_PC:     CPU_ReadPC(CPU); break;
        case READ_ZPG:    CPU_ReadZpg(CPU); break;
        case READ_ADDR:   CPU_ReadAddr(CPU); break;
        case READ_ADDR_Y: CPU_ReadAddrY(CPU); break;
        case READ_STACK:  CPU_ReadStack(CPU); break;
        case READ_WRAP:  CPU_ReadWrap(CPU); break;
        // WRITE
        case WRITE_DATA: CPU_Write(CPU); break;
        // PULL
        case PULL_HI_PC: CPU_PullHiPC(CPU); break;
        case PULL_LO_PC: CPU_PullLoPC(CPU); break;
        case PULL_SR:    CPU_PullSR(CPU); break;
        case PULL_A:     CPU_PullA(CPU); break;
        // PUSH
        case PUSH_HI_PC:  CPU_PushHiPC(CPU); break;
        case PUSH_LO_PC:  CPU_PushLoPC(CPU); break;
        case PUSH_SR_BRK: CPU_PushSRBRK(CPU); break;
        case PUSH_SR_IRQ: CPU_PushSRIRQ(CPU); break;
        case PUSH_SR:     CPU_PushSR(CPU); break;
        case PUSH_A:      CPU_PushA(CPU); break;
        // JSR
        case JSR_BEGIN: CPU_JSRBegin(CPU);break;
        case JSR_HI_PC: CPU_JSRHiPC(CPU);break;
        case JSR_LO_PC: CPU_JSRLoPC(CPU);break;
        case JSR_END:   CPU_JSREnd(CPU);break;
        // ADDR
        case ADDR_ADD_X:   CPU_AddrAddX(CPU); break;
        case ADDR_ADD_Y:   CPU_AddrAddY(CPU); break;
        case ADDR_ADD_X_C: CPU_AddrAddXC(CPU); break;
        case ADDR_ADD_Y_C: CPU_AddrAddYC(CPU); break;
        case ADDR_CARRY:   CPU_AddrCarry(CPU); break;
        // REG
        case ADC: CPU_ADC(CPU); break;
        case AND: CPU_AND(CPU); break;
        case BIT: CPU_BIT(CPU); break;
        case CMP: CPU_CMP(CPU); break;
        case CPX: CPU_CPX(CPU); break;
        case CPY: CPU_CPY(CPU); break;
        case EOR: CPU_EOR(CPU); break;
        case LDA: CPU_LDA(CPU); break;
        case LDX: CPU_LDX(CPU); break;
        case LDY: CPU_LDY(CPU); break;
        case ORA: CPU_ORA(CPU); break;
        case SBC: CPU_SBC(CPU); break;
        case STA: CPU_STA(CPU); break;
        case STX: CPU_STX(CPU); break;
        case STY: CPU_STY(CPU); break;
        case DEX: CPU_DEX(CPU); break;
        case DEY: CPU_DEY(CPU); break;
        case INX: CPU_INX(CPU); break;
        case INY: CPU_INY(CPU); break;
        // RMW
        case ASL:     CPU_ASL(CPU); break;
        case ASL_ACC: CPU_ASL_ACC(CPU); break;
        case DEC:     CPU_DEC(CPU); break;
        case INC:     CPU_INC(CPU); break;
        case LSR:     CPU_LSR(CPU);break;
        case LSR_ACC: CPU_LSR_ACC(CPU);break;
        case ROL:     CPU_ROL(CPU);break;
        case ROL_ACC: CPU_ROL_ACC(CPU);break;
        case ROR:     CPU_ROR(CPU);break;
        case ROR_ACC: CPU_ROR_ACC(CPU);break;
        // BRANCH
        case BCC: CPU_BCC(CPU); break;
        case BCS: CPU_BCS(CPU); break;
        case BEQ: CPU_BEQ(CPU); break;
        case BMI: CPU_BMI(CPU); break;
        case BNE: CPU_BNE(CPU); break;
        case BPL: CPU_BPL(CPU); break;
        case BVC: CPU_BVC(CPU); break;
        case BVS: CPU_BVS(CPU); break;
        case BRANCH_LO_PC: CPU_BranchLo(CPU); break;
        case BRANCH_HI_PC: CPU_BranchHi(CPU); break;
        // FLAG
        case CLC: CPU_CLC(CPU); break;
        case CLD: CPU_CLD(CPU); break;
        case CLI: CPU_CLI(CPU); break;
        case CLV: CPU_CLV(CPU); break;
        case SEC: CPU_SEC(CPU); break;
        case SED: CPU_SED(CPU); break;
        case SEI: CPU_SEI(CPU); break;
        // TRANSFER
        case TAX: CPU_TAX(CPU); break;
        case TAY: CPU_TAY(CPU); break;
        case TSX: CPU_TSX(CPU); break;
        case TXA: CPU_TXA(CPU); break;
        case TXS: CPU_TXS(CPU); break;
        case TYA: CPU_TYA(CPU); break;
        default: Assert(0);
    }
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
    CPU->SR |= STATUS_INTERRUPT;

    CPU->Data = 0;
    CPU->Address = 0x4015;
    CPU_Write(CPU);

    CPU->Address = 0xFFFC;
    CPU_ReadNext(CPU);
    CPU_ReadPC(CPU);
    CPU_FetchOP(CPU);
}

static void
CPU_Power(cpu *CPU)
{
    CPU->A = 0;
    CPU->X = 0;
    CPU->Y = 0;
    CPU->SP = 0; 
    CPU->SR = 0;
    CPU_Reset(CPU);
}

static cpu*
CPU_Create(console *Console)
{
    cpu *CPU = (cpu *)Api_Malloc(sizeof(cpu));
    CPU->Console = Console;    
    return CPU;
}
