
struct cpu_op
{
    u8 Cycles;
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
    FETCH_NEXT_OP = 0,
    FETCH_DATA,
    FETCH_LO_PC,
    FETCH_HI_PC,
    FETCH_LO_ADDR,
    FETCH_HI_ADDR,
    FETCH_ZPG_ADDR,
    FETCH_HI_ADDR_X,
    FETCH_HI_ADDR_Y,
    FETCH_HI_PC_AND_LO_ADDR,
    // READ
    READ_DATA,
    READ_LO_PC,
    READ_HI_PC,
    READ_LO_ADDR,
    READ_HI_ADDR,
    READ_HI_ADDR_Y,
    READ_STACK,
    // WRITE
    WRITE_DATA,
    // ADDR
    ADDR_ADD_X,
    ADDR_ADD_Y,
    ADDR_ADD_X_C,
    ADDR_HI_CARRY,
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
    // STACK
    PUSH_HI_PC,
    PUSH_LO_PC,
    PUSH_SR_BRK,
    PUSH_SR_IRQ,
    PUSH_SR,
    PUSH_A,
    PULL_DATA,
    PULL_HI_PC,
    PULL_LO_PC,
    PULL_SR,
    PULL_A,
    // TRANSFER
    TAX,
    TAY,
    TSX,
    TXA,
    TXS,
    TYA,
};

#define REG_OP_imm(OP)  { 2, OP,             FETCH_NEXT_OP }
#define REG_OP_zpg(OP)  { 3, FETCH_ZPG_ADDR, OP,              FETCH_NEXT_OP }
#define REG_OP_zpgX(OP) { 4, FETCH_ZPG_ADDR, ADDR_ADD_X,      OP,             FETCH_NEXT_OP }
#define REG_OP_zpgY(OP) { 4, FETCH_ZPG_ADDR, ADDR_ADD_Y,      OP,             FETCH_NEXT_OP }
#define REG_OP_abs(OP)  { 4, FETCH_LO_ADDR,  FETCH_HI_ADDR,   OP,             FETCH_NEXT_OP }
#define REG_OP_absX(OP) { 5, FETCH_LO_ADDR,  FETCH_HI_ADDR_X, ADDR_HI_CARRY,  OP,            FETCH_NEXT_OP } //
#define REG_OP_absY(OP) { 5, FETCH_LO_ADDR,  FETCH_HI_ADDR_Y, ADDR_HI_CARRY,  OP,            FETCH_NEXT_OP } //
#define REG_OP_indX(OP) { 6, FETCH_LO_ADDR,  ADDR_ADD_X,      READ_LO_ADDR,   READ_HI_ADDR,  OP,           FETCH_NEXT_OP }
#define REG_OP_indY(OP) { 6, FETCH_ZPG_ADDR, READ_LO_ADDR,    READ_HI_ADDR_Y, ADDR_HI_CARRY, OP,           FETCH_NEXT_OP } //

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

#define STA_imm  { }
#define STA_zpg  REG_OP_zpg (STA)
#define STA_zpgX REG_OP_zpgX(STA)
#define STA_abs  REG_OP_abs (STA)
#define STA_absX REG_OP_absX(STA)
#define STA_absY REG_OP_absY(STA)
#define STA_indX REG_OP_indX(STA)
#define STA_indY REG_OP_indY(STA)

#define STX_imm  { }
#define STX_zpg  REG_OP_zpg (STX)
#define STX_zpgY REG_OP_zpgY(STX)
#define STX_abs  REG_OP_abs (STX)
#define STX_absX { }
#define STX_absY { }
#define STX_indX { }
#define STX_indY { }

#define STY_imm  { }
#define STY_zpg  REG_OP_zpg (STY)
#define STY_zpgX REG_OP_zpgX(STY)
#define STY_abs  REG_OP_abs (STY)
#define STY_absX { }
#define STY_absY { }
#define STY_indX { }
#define STY_indY { }

#define DEX_imp { 2, DEX, FETCH_NEXT_OP }
#define DEY_imp { 2, DEY, FETCH_NEXT_OP }

#define INX_imp { 2, INX, FETCH_NEXT_OP }
#define INY_imp { 2, INY, FETCH_NEXT_OP }

#define RMW_OP_acc(OP)  { 2, OP##_ACC,       FETCH_NEXT_OP }
#define RMW_OP_zpg(OP)  { 5, FETCH_ZPG_ADDR, READ_DATA,     OP,           WRITE_DATA, FETCH_NEXT_OP }
#define RMW_OP_zpgX(OP) { 6, FETCH_ZPG_ADDR, ADDR_ADD_X,    READ_DATA,    OP,         WRITE_DATA,   FETCH_NEXT_OP }
#define RMW_OP_abs(OP)  { 6, FETCH_LO_ADDR,  FETCH_HI_ADDR, READ_DATA,    OP,         WRITE_DATA,   FETCH_NEXT_OP }
#define RMW_OP_absX(OP) { 7, FETCH_LO_ADDR,  FETCH_HI_ADDR, ADDR_ADD_X_C, READ_DATA,  OP,           WRITE_DATA,   FETCH_NEXT_OP }

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

#define BCC_rel { 4, BCC, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BCS_rel { 4, BCS, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BEQ_rel { 4, BEQ, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BMI_rel { 4, BMI, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BNE_rel { 4, BNE, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BPL_rel { 4, BPL, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BVC_rel { 4, BVC, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }
#define BVS_rel { 4, BVS, BRANCH_LO_PC, BRANCH_HI_PC, FETCH_NEXT_OP }

#define CLC_imp { 2, CLC, FETCH_NEXT_OP }
#define CLD_imp { 2, CLD, FETCH_NEXT_OP }
#define CLI_imp { 2, CLI, FETCH_NEXT_OP }
#define CLV_imp { 2, CLV, FETCH_NEXT_OP }

#define SEC_imp { 2, SEC, FETCH_NEXT_OP }
#define SED_imp { 2, SED, FETCH_NEXT_OP }
#define SEI_imp { 2, SEI, FETCH_NEXT_OP }

#define BRK_imp { 7, READ_DATA, PUSH_HI_PC, PUSH_LO_PC, PUSH_SR_BRK, FETCH_LO_PC, FETCH_HI_PC, FETCH_NEXT_OP }
#define IRQ_imp { 7, READ_DATA, PUSH_HI_PC, PUSH_LO_PC, PUSH_SR_IRQ, FETCH_LO_PC, FETCH_HI_PC, FETCH_NEXT_OP }

#define JMP_abs { 3, FETCH_LO_PC,   FETCH_HI_PC,   FETCH_NEXT_OP }
#define JMP_ind { 5, FETCH_LO_ADDR, FETCH_HI_ADDR, READ_LO_PC,   READ_HI_PC, FETCH_NEXT_OP }
#define JSR_abs { 6, FETCH_LO_ADDR, READ_STACK,    PUSH_HI_PC,   PUSH_LO_PC, FETCH_HI_PC_AND_LO_ADDR, FETCH_NEXT_OP }

#define NOP_imp { 2, READ_DATA, FETCH_NEXT_OP }

#define PHA_imp { 3, READ_DATA, PUSH_A,  FETCH_NEXT_OP }
#define PHP_imp { 3, READ_DATA, PUSH_SR, FETCH_NEXT_OP }

#define PLA_imp { 4, READ_DATA, PULL_DATA, PULL_A,  FETCH_NEXT_OP }
#define PLP_imp { 4, READ_DATA, PULL_DATA, PULL_SR, FETCH_NEXT_OP }

#define RTI_imp { 6, PULL_DATA, PULL_SR,   PULL_LO_PC, PULL_HI_PC, READ_DATA, FETCH_NEXT_OP }
#define RTS_imp { 6, READ_DATA, PULL_DATA, PULL_LO_PC, PULL_HI_PC, FETCH_DATA, FETCH_NEXT_OP }

#define TAX_imp { 2, TAX, FETCH_NEXT_OP }
#define TAY_imp { 2, TAY, FETCH_NEXT_OP }
#define TSX_imp { 2, TSX, FETCH_NEXT_OP }
#define TXA_imp { 2, TXA, FETCH_NEXT_OP }
#define TXS_imp { 2, TXS, FETCH_NEXT_OP }
#define TYA_imp { 2, TYA, FETCH_NEXT_OP }

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
    CPU->Address = (u16)(Value & 0x7) << 8;
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
    if (Cycle <= 2)
        return;
    
    if (Cycle & 1)
        CPU->Data = CPU_R(CPU, CPU->Address);
    else
        CPU_W(CPU, OAMDATA, CPU->Data);

    if (Cycle >= 514)
        Cycle = false;
}

// READS FROM PC LOCATION

inline void
CPU_Fetch(cpu *CPU)
{
    CPU->Data = CPU_R(CPU, CPU->PC++);
}

inline void
CPU_FetchLoPC(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->PC = (CPU->PC & 0xFF00) | CPU->Data;
}

inline void
CPU_FetchHiPC(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->PC = (CPU->PC & 0xFF) | (CPU->Data << 8);
}

inline void
CPU_FetchLoAddr(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->Address = (CPU->Address & 0xFF00) | CPU->Data;
}

inline void
CPU_FetchHiAddr(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->Address = (CPU->Address & 0xFF) | (CPU->Data << 8);
}

inline void
CPU_FetchHiAddrX(cpu *CPU)
{
    CPU_Fetch(CPU);
    u16 NewAddress = CPU->Address + CPU->X;
    CPU->Address = (CPU->Data << 8) | ((CPU->X + CPU->Address) & 0xFF);
    if (CPU->Address == NewAddress)
        ++CPU->OPCycle;
}

inline void
CPU_FetchHiAddrY(cpu *CPU)
{
    CPU_Fetch(CPU);
    u16 NewAddress = CPU->Address + CPU->Y;
    CPU->Address = (CPU->Data << 8) | ((CPU->Y + CPU->Address) & 0xFF);
    if (CPU->Address == NewAddress)
        ++CPU->OPCycle;
}

inline void
CPU_FetchZpgAddr(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->Address = CPU->Data;
}

inline void
CPU_FetchHiPcAndLoAddr(cpu *CPU)
{
    CPU_Fetch(CPU);
    CPU->PC = (CPU->Address & 0xFF) | (CPU->Data << 8);
}

// READS FROM ADDRESS BUS LOCATION

inline void
CPU_Read(cpu *CPU)
{
    CPU->Data = CPU_R(CPU, CPU->Address++);
}

inline void
CPU_ReadLoPC(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->PC = (CPU->PC & 0xFF00) | CPU->Data;
}

inline void
CPU_ReadHiPC(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->PC = (CPU->PC & 0xFF) | (CPU->Data << 8);
}

inline void
CPU_ReadLoAddr(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Address = (CPU->Address & 0xFF00) | CPU->Data;
}

inline void
CPU_ReadHiAddr(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Address = (CPU->Address & 0xFF) | (CPU->Data << 8);
}

inline void
CPU_ReadHiAddrY(cpu *CPU)
{
    CPU_Read(CPU);
    u16 NewAddress = CPU->Address + CPU->Y;
    CPU->Address = (CPU->Data << 8) | ((CPU->Y + CPU->Address) & 0xFF);
    if (CPU->Address == NewAddress)
        ++CPU->OPCycle;
}

inline void
CPU_ReadA(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A = CPU->Data;
}

inline void
CPU_ReadX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->X = CPU->Data;
}

inline void
CPU_ReadY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Y = CPU->Data;
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

// ADDRESS

inline void
CPU_AddressAddX(cpu *CPU)
{
    CPU_Read(CPU);
    u16 NewAddress = CPU->Address + CPU->X;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0x00FF);
}

inline void
CPU_AddressAddY(cpu *CPU)
{
    CPU_Read(CPU);
    u16 NewAddress = CPU->Address + CPU->Y;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0x00FF);
}

inline void
CPU_AddressAddXC(cpu *CPU)
{
    u16 NewAddress = CPU->Address + CPU->X;
    CPU->Address = (CPU->Address & 0xFF00) | (NewAddress & 0x00FF);
    CPU_Read(CPU);
    CPU->Address = NewAddress;
}

inline void
CPU_AddrHiCarry(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Address += 0x100;
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

// REG OPS

inline void
CPU_ADC(cpu *CPU)
{
    CPU_Read(CPU);
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
    CPU->A &= CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_BIT(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNVZ(CPU);
    CPU->SR |= CPU->Data & (STATUS_NEGATIVE | STATUS_OVERFLOW);
    if ((CPU->A & CPU->Data) == 0) CPU_SetZ(CPU);
}

inline void
CPU_CMP(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZC(CPU);
    if (CPU->A >= CPU->Data) CPU_SetC(CPU);
    if (CPU->A == CPU->Data) CPU_SetZ(CPU);
    if ((u8)(CPU->A - CPU->Data) & 0x80) CPU_SetN(CPU);
}

inline void
CPU_CPX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZC(CPU);
    if (CPU->X >= CPU->Data) CPU_SetC(CPU);
    if (CPU->X == CPU->Data) CPU_SetZ(CPU);
    if ((u8)(CPU->X - CPU->Data) & 0x80) CPU_SetN(CPU);
}

inline void
CPU_CPY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU_ClearNZC(CPU);
    if (CPU->Y >= CPU->Data) CPU_SetC(CPU);
    if (CPU->Y == CPU->Data) CPU_SetZ(CPU);
    if ((u8)(CPU->Y - CPU->Data) & 0x80) CPU_SetN(CPU);
}

inline void
CPU_EOR(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A ^= CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_LDA(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}
inline void
CPU_LDX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->X = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}
inline void
CPU_LDY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Y = CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->Y == 0) CPU_SetZ(CPU);
    if (CPU->Y & 0x80) CPU_SetN(CPU);
}

inline void
CPU_ORA(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A |= CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->A == 0) CPU_SetZ(CPU);
    if (CPU->A & 0x80) CPU_SetN(CPU);
}

inline void
CPU_SBC(cpu *CPU)
{
    CPU_Read(CPU);
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
    --CPU->X;
    CPU_ClearNZ(CPU);
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}

inline void
CPU_DEY(cpu *CPU)
{
    CPU_Read(CPU);
    --CPU->Y;
    CPU_ClearNZ(CPU);
    if (CPU->Y == 0) CPU_SetZ(CPU);
    if (CPU->Y & 0x80) CPU_SetN(CPU);
}

inline void
CPU_INX(cpu *CPU)
{
    CPU_Read(CPU);
    ++CPU->X;
    CPU_ClearNZ(CPU);
    if (CPU->X == 0) CPU_SetZ(CPU);
    if (CPU->X & 0x80) CPU_SetN(CPU);
}

inline void
CPU_INY(cpu *CPU)
{
    CPU_Read(CPU);
    ++CPU->Y;
    CPU_ClearNZ(CPU);
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
    --CPU->Data;
    CPU_ClearNZ(CPU);
    if (CPU->Data == 0) CPU_SetZ(CPU);
    if (CPU->Data & 0x80) CPU_SetN(CPU);
}

inline void
CPU_INC(cpu *CPU)
{
    CPU_Write(CPU);
    ++CPU->Data;
    CPU_ClearNZ(CPU);
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
CPU_BCC(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (!(CPU->SR & STATUS_CARRY))
        CPU->OPCycle = 3;
}

inline void
CPU_BCS(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (CPU->SR & STATUS_CARRY)
        CPU->OPCycle = 3; 
}

inline void
CPU_BEQ(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (CPU->SR & STATUS_ZERO)
        CPU->OPCycle = 3; 
}

inline void
CPU_BMI(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (CPU->SR & STATUS_NEGATIVE)
        CPU->OPCycle = 3; 
}

inline void
CPU_BNE(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (!(CPU->SR & STATUS_ZERO))
        CPU->OPCycle = 3; 
}

inline void
CPU_BPL(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (!(CPU->SR & STATUS_NEGATIVE))
        CPU->OPCycle = 3; 
}

inline void
CPU_BVC(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (!(CPU->SR & STATUS_OVERFLOW))
        CPU->OPCycle = 3; 
}

inline void
CPU_BVS(cpu *CPU)
{
    CPU_Fetch(CPU);
    if (CPU->SR & STATUS_OVERFLOW)
        CPU->OPCycle = 3; 
}

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
CPU_Push(cpu *CPU)
{
    CPU->Address = CPU->SP-- | 0x100;
    CPU_Write(CPU);
}

inline void
CPU_PushHiPC(cpu *CPU)
{
    CPU->Data = CPU->PC >> 8;
    CPU_Push(CPU);
}

inline void
CPU_PushLoPC(cpu *CPU)
{
    CPU->Data = CPU->PC & 0xFF;
    CPU_Push(CPU);
}

inline void
CPU_PushSRBRK(cpu *CPU)
{
    CPU->Data = CPU->SR | STATUS_ | STATUS_BREAK;
    CPU_Push(CPU);
    CPU->Address = 0xFFFA;
    CPU->Interrupt = false;
}

inline void
CPU_PushSRIRQ(cpu *CPU)
{
    CPU->Data = CPU->SR | STATUS_;
    CPU_Push(CPU);
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
    CPU->Data = CPU->SR | STATUS_;
    CPU_Push(CPU);
}

inline void
CPU_PushA(cpu *CPU)
{
    CPU->Data = CPU->A;
    CPU_Push(CPU);
}

inline void
CPU_Pull(cpu *CPU)
{
    CPU->Address = CPU->SP++ | 0x100;
    CPU_Read(CPU);
}

inline void
CPU_PullHiPC(cpu *CPU)
{
    CPU_Pull(CPU);
    CPU->PC = (CPU->PC & 0xFF) | (CPU->Data << 8);
}

inline void
CPU_PullLoPC(cpu *CPU)
{
    CPU_Pull(CPU);
    CPU->PC = (CPU->PC & 0xFF00) | CPU->Data;
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
}

inline void
CPU_TAX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->X = CPU->A;
}

inline void
CPU_TAY(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->Y = CPU->A;
}

inline void
CPU_TSX(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->X = CPU->SP;
}

inline void
CPU_TXA(cpu *CPU)
{
    CPU_Read(CPU);
    CPU->A = CPU->X;
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
}

inline void
CPU_FetchNextOP(cpu *CPU)
{
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

    if ((CPU->OP.Cycles - 2) ==  CPU->OPCycle)
        if (CPU->NMIOccurred || CPU->IRQOccurred)
            CPU->Interrupt = true;

    switch (CPU->OP.CycleOp[CPU->OPCycle++])
    {
        // FETCH
        case FETCH_NEXT_OP:   CPU_FetchNextOP(CPU); break;
        case FETCH_DATA:      CPU_Fetch(CPU); break;
        case FETCH_LO_PC:     CPU_FetchLoPC(CPU); break;
        case FETCH_HI_PC:     CPU_FetchHiPC(CPU); break;
        case FETCH_LO_ADDR:   CPU_FetchLoAddr(CPU); break;
        case FETCH_HI_ADDR:   CPU_FetchHiAddr(CPU); break;
        case FETCH_ZPG_ADDR:  CPU_FetchZpgAddr(CPU); break;
        case FETCH_HI_ADDR_X: CPU_FetchHiAddrX(CPU); break;
        case FETCH_HI_ADDR_Y: CPU_FetchHiAddrY(CPU); break;
        case FETCH_HI_PC_AND_LO_ADDR: CPU_FetchHiPcAndLoAddr(CPU); break;
        // READ
        case READ_DATA:    CPU_Read(CPU); break;
        case READ_LO_PC:   CPU_ReadLoPC(CPU); break;
        case READ_HI_PC:   CPU_ReadHiPC(CPU); break;
        case READ_LO_ADDR: CPU_ReadLoAddr(CPU); break;
        case READ_HI_ADDR: CPU_ReadHiAddr(CPU); break;
        case READ_HI_ADDR_Y: CPU_ReadHiAddr(CPU); break;
        case READ_STACK:   CPU_ReadStack(CPU); break;
        // WRITE
        case WRITE_DATA: CPU_Write(CPU); break;
        // ADDR
        case ADDR_ADD_X:    CPU_AddressAddX(CPU); break;
        case ADDR_ADD_Y:    CPU_AddressAddY(CPU); break;
        case ADDR_ADD_X_C:  CPU_AddressAddXC(CPU); break;
        case ADDR_HI_CARRY: CPU_AddrHiCarry(CPU); break;
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
        // STACK
        case PUSH_HI_PC:  CPU_PushHiPC(CPU); break;
        case PUSH_LO_PC:  CPU_PushLoPC(CPU); break;
        case PUSH_SR_BRK: CPU_PushSRBRK(CPU); break;
        case PUSH_SR_IRQ: CPU_PushSRIRQ(CPU); break;
        case PUSH_SR:     CPU_PushSR(CPU); break;
        case PUSH_A:      CPU_PushA(CPU); break;
        case PULL_DATA:   CPU_Pull(CPU); break;
        case PULL_HI_PC:  CPU_PushHiPC(CPU); break;
        case PULL_LO_PC:  CPU_PushLoPC(CPU); break;
        case PULL_SR:     CPU_PushSR(CPU); break;
        case PULL_A:      CPU_PushA(CPU); break;
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

    CPU->Address = 0xFFFC;
    CPU_ReadLoPC(CPU);
    CPU_ReadHiPC(CPU);
    CPU_FetchNextOP(CPU);
    
    CPU->Data = 0;
    CPU->Address = 0x4015;
    CPU_Write(CPU);
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
