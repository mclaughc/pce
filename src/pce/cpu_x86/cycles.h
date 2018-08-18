#pragma once
#include "types.h"

namespace CPU_X86 {

enum CYCLE_GROUP : uint32
{
  // For two-operand groups, the order is dst, src.
  // Order is imm, mem, reg. Therefore, to get from the base (reg) r/m, if r is set, add one.
  // Immediate instructions use the same number of cycles as register operands.
  // For variable-timing instructions (e.g. mul/div), the fewest number of cycles is used.
  CYCLES_MOV_RM_MEM_REG,
  CYCLES_MOV_RM_REG_REG,
  CYCLES_MOV_REG_RM_MEM,
  CYCLES_MOV_REG_RM_REG,
  CYCLES_MOV_REG_IMM,
  CYCLES_MOV_REG_MEM,
  CYCLES_MOV_SREG_RM_MEM,
  CYCLES_MOV_SREG_RM_MEM_PMODE,
  CYCLES_MOV_SREG_RM_REG,
  CYCLES_MOV_SREG_RM_REG_PMODE,
  CYCLES_MOV_RM_MEM_SREG,
  CYCLES_MOV_RM_REG_SREG,
  CYCLES_MOVSX_REG_RM_MEM,
  CYCLES_MOVSX_REG_RM_REG,
  CYCLES_MOVZX_REG_RM_REG,
  CYCLES_MOVZX_REG_RM_MEM,
  CYCLES_XCHG_REG_RM_MEM,
  CYCLES_XCHG_REG_RM_REG,
  CYCLES_IN_IMM,
  CYCLES_IN_IMM_PMODE,
  CYCLES_IN_EDX,
  CYCLES_IN_EDX_PMODE,
  CYCLES_OUT_IMM,
  CYCLES_OUT_IMM_PMODE,
  CYCLES_OUT_EDX,
  CYCLES_OUT_EDX_PMODE,
  CYCLES_LEA,
  CYCLES_LxS,
  CYCLES_LxS_PMODE,
  CYCLES_CLEAR_SET_FLAG,
  CYCLES_CLI, // also sti
  CYCLES_CLTS,
  CYCLES_LAHF,
  CYCLES_SAHF,
  CYCLES_PUSHF,
  CYCLES_POPF,
  CYCLES_PUSHA,
  CYCLES_POPA,
  CYCLES_PUSH_MEM,
  CYCLES_PUSH_REG,
  CYCLES_PUSH_SREG,
  CYCLES_PUSH_IMM,
  CYCLES_POP_MEM,
  CYCLES_POP_REG,
  CYCLES_POP_SREG,
  CYCLES_POP_SREG_PMODE,
  CYCLES_BCD_ADDSUB,
  CYCLES_AAD,
  CYCLES_AAM,
  CYCLES_CBW,
  CYCLES_CWD,
  CYCLES_XLAT,
  CYCLES_ALU_REG_IMM, // add, adc, sub, sbb, ...
  CYCLES_ALU_REG_RM_MEM,
  CYCLES_ALU_REG_RM_REG,
  CYCLES_ALU_RM_MEM_REG,
  CYCLES_ALU_RM_REG_REG,
  CYCLES_CMP_REG_IMM, // add, adc, sub, sbb, ...
  CYCLES_CMP_REG_RM_MEM,
  CYCLES_CMP_REG_RM_REG,
  CYCLES_CMP_RM_MEM_REG,
  CYCLES_CMP_RM_REG_REG,
  CYCLES_INC_RM_MEM, // also dec
  CYCLES_INC_RM_REG, // also dec
  CYCLES_NEG_RM_MEM, // also not
  CYCLES_NEG_RM_REG, // also not
  CYCLES_TEST_REG_RM_MEM,
  CYCLES_TEST_REG_RM_REG,
  CYCLES_TEST_RM_MEM_REG,
  CYCLES_TEST_RM_REG_REG,
  CYCLES_MUL_8_RM_MEM,
  CYCLES_MUL_8_RM_REG,
  CYCLES_MUL_16_RM_MEM,
  CYCLES_MUL_16_RM_REG,
  CYCLES_MUL_32_RM_MEM,
  CYCLES_MUL_32_RM_REG,
  CYCLES_IMUL_8_RM_MEM,
  CYCLES_IMUL_8_RM_REG,
  CYCLES_IMUL_16_RM_MEM,
  CYCLES_IMUL_16_RM_REG,
  CYCLES_IMUL_16_REG_RM_MEM,
  CYCLES_IMUL_16_REG_RM_REG,
  CYCLES_IMUL_32_RM_MEM,
  CYCLES_IMUL_32_RM_REG,
  CYCLES_IMUL_32_REG_RM_MEM,
  CYCLES_IMUL_32_REG_RM_REG,
  CYCLES_DIV_8_RM_MEM,
  CYCLES_DIV_8_RM_REG,
  CYCLES_DIV_16_RM_MEM,
  CYCLES_DIV_16_RM_REG,
  CYCLES_DIV_32_RM_MEM,
  CYCLES_DIV_32_RM_REG,
  CYCLES_IDIV_8_RM_MEM,
  CYCLES_IDIV_8_RM_REG,
  CYCLES_IDIV_16_RM_MEM,
  CYCLES_IDIV_16_RM_REG,
  CYCLES_IDIV_32_RM_MEM,
  CYCLES_IDIV_32_RM_REG,
  CYCLES_ROL_RM_MEM,  // also ROR
  CYCLES_ROL_RM_REG,  // also ROR
  CYCLES_RCL_RM_MEM,  // also RCR
  CYCLES_RCL_RM_REG,  // also RCR
  CYCLES_SHLD_RM_MEM, // also SHRD
  CYCLES_SHLD_RM_REG, // also SHRD
  CYCLES_CMPS,
  CYCLES_INS,
  CYCLES_INS_PMODE,
  CYCLES_LODS,
  CYCLES_MOVS,
  CYCLES_OUTS,
  CYCLES_OUTS_PMODE,
  CYCLES_SCAS,
  CYCLES_STOS,
  CYCLES_REP_CMPS_BASE,
  CYCLES_REP_INS_BASE,
  CYCLES_REP_INS_PMODE_BASE,
  CYCLES_REP_LODS_BASE,
  CYCLES_REP_MOVS_BASE,
  CYCLES_REP_OUTS_BASE,
  CYCLES_REP_OUTS_PMODE_BASE,
  CYCLES_REP_SCAS_BASE,
  CYCLES_REP_STOS_BASE,
  CYCLES_REP_CMPS_N,
  CYCLES_REP_INS_N,
  CYCLES_REP_INS_PMODE_N,
  CYCLES_REP_LODS_N,
  CYCLES_REP_MOVS_N,
  CYCLES_REP_OUTS_N,
  CYCLES_REP_OUTS_PMODE_N,
  CYCLES_REP_SCAS_N,
  CYCLES_REP_STOS_N,
  CYCLES_BOUND_FAIL,
  CYCLES_BOUND_SUCCESS,
  CYCLES_BSWAP,
  CYCLES_CMPXCHG,
  CYCLES_CMPXCHG8B,
  CYCLES_CMOV,
  CYCLES_XADD,
  CYCLES_INVD, // also WBINVD
  CYCLES_INVLPG,
  CYCLES_CPUID,
  CYCLES_RDTSC,
  CYCLES_RSM,
  CYCLES_RDMSR,    // also WRMSR
  CYCLES_BSF_BASE, // also BSR
  CYCLES_BSF_N,    // also BSR
  CYCLES_BT_RM_MEM_REG,
  CYCLES_BT_RM_REG_REG,
  CYCLES_BT_RM_MEM_IMM,
  CYCLES_BT_RM_REG_IMM,
  CYCLES_BTx_RM_MEM_REG, // BTC, BTR, BTS
  CYCLES_BTx_RM_REG_REG, // BTC, BTR, BTS
  CYCLES_BTx_RM_MEM_IMM, // BTC, BTR, BTS
  CYCLES_BTx_RM_REG_IMM, // BTC, BTR, BTS
  CYCLES_Jcc_NOT_TAKEN,
  CYCLES_Jcc_TAKEN,
  CYCLES_JCXZ_NOT_TAKEN,
  CYCLES_JCXZ_TAKEN,
  CYCLES_LOOP,
  CYCLES_LOOPZ, // also loopnz
  CYCLES_SETcc_RM_MEM,
  CYCLES_SETcc_RM_REG,
  CYCLES_ENTER,
  CYCLES_LEAVE,
  CYCLES_CALL_NEAR,
  CYCLES_CALL_NEAR_RM_MEM,
  CYCLES_CALL_NEAR_RM_REG,
  CYCLES_JMP_NEAR,
  CYCLES_JMP_NEAR_RM_MEM,
  CYCLES_JMP_NEAR_RM_REG,
  CYCLES_CALL_FAR,
  CYCLES_CALL_FAR_PMODE,
  CYCLES_CALL_FAR_PTR,
  CYCLES_CALL_FAR_PTR_PMODE,
  CYCLES_JMP_FAR,
  CYCLES_JMP_FAR_PMODE,
  CYCLES_JMP_FAR_PTR,
  CYCLES_JMP_FAR_PTR_PMODE,
  CYCLES_RET_NEAR,
  CYCLES_RET_FAR,
  CYCLES_RET_FAR_PMODE,
  CYCLES_INT,
  CYCLES_INT3,
  CYCLES_INTO_FALSE,
  CYCLES_INTO_TRUE,
  CYCLES_IRET,
  CYCLES_IRET_PMODE,
  CYCLES_HLT,
  CYCLES_NOP,
  CYCLES_WAIT,
  CYCLES_MOV_CR0_REG,
  CYCLES_MOV_CR2_REG,
  CYCLES_MOV_CR3_REG,
  CYCLES_MOV_REG_CR,
  CYCLES_MOV_DR0_3_REG,
  CYCLES_MOV_DR6_7_REG,
  CYCLES_MOV_REG_DR0_3,
  CYCLES_MOV_REG_DR6_7,
  CYCLES_ARPL_RM_MEM,
  CYCLES_ARPL_RM_REG,
  CYCLES_LAR_RM_MEM,
  CYCLES_LAR_RM_REG,
  CYCLES_LSL_RM_MEM,
  CYCLES_LSL_RM_REG,
  CYCLES_LTR_RM_MEM,
  CYCLES_LTR_RM_REG,
  CYCLES_LGDT, // also LIDT
  CYCLES_LLDT_RM_MEM,
  CYCLES_LLDT_RM_REG,
  CYCLES_LMSW_RM_MEM,
  CYCLES_LMSW_RM_REG,
  CYCLES_SMSW_RM_MEM,
  CYCLES_SMSW_RM_REG,
  CYCLES_SGDT, // also LIDT
  CYCLES_SLDT_RM_MEM,
  CYCLES_SLDT_RM_REG,
  CYCLES_STR_RM_MEM,
  CYCLES_STR_RM_REG,
  CYCLES_VERR_RM_MEM,
  CYCLES_VERR_RM_REG,
  CYCLES_VERW_RM_MEM,
  CYCLES_VERW_RM_REG,
  CYCLES_F2XM1,
  CYCLES_FABS,
  CYCLES_FADD,
  CYCLES_FBLD,
  CYCLES_FBSTP,
  CYCLES_FCHS,
  CYCLES_FCOM,
  CYCLES_FDECSTP,
  CYCLES_FDIV,
  CYCLES_FFREE,
  CYCLES_FIADD,
  CYCLES_FICOM,
  CYCLES_FIDIV,
  CYCLES_FILD,
  CYCLES_FIMUL,
  CYCLES_FINCSTP,
  CYCLES_FIST,
  CYCLES_FISUB,
  CYCLES_FLD,
  CYCLES_FLD_CONSTANT,      // FLDZ, FLD1
  CYCLES_FLD_LONG_CONSTANT, // FLD2E, FLD2T, FLDLG2, FLDLN2, FLDPI
  CYCLES_FLDCW,
  CYCLES_FLDENV,
  CYCLES_FMUL,
  CYCLES_FNCLEX,
  CYCLES_FNDISI,
  CYCLES_FNENI,
  CYCLES_FNINIT,
  CYCLES_FNSAVE,
  CYCLES_FNSTCW,
  CYCLES_FNSTENV,
  CYCLES_FNSTSW,
  CYCLES_FNOP,
  CYCLES_FPATAN,
  CYCLES_FPREM,
  CYCLES_FPTAN,
  CYCLES_FRNDINT,
  CYCLES_FRSTOR,
  CYCLES_FSCALE,
  CYCLES_FSQRT,
  CYCLES_FST,
  CYCLES_FSUB,
  CYCLES_FTST,
  CYCLES_FXAM,
  CYCLES_FXCH,
  CYCLES_FXTRACT,
  CYCLES_FYL2X,
  CYCLES_FYL2XP1,
  CYCLES_FSETPM,
  CYCLES_FCOS,
  CYCLES_FPREM1,
  CYCLES_FSIN,
  CYCLES_FSINCOS,
  CYCLES_FUCOM,
  NUM_CYCLE_GROUPS
};

extern const uint32 g_cycle_group_timings[NUM_CYCLE_GROUPS][3];
} // namespace CPU_X86
