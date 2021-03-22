#pragma once
#include<iostream>

enum regNo {
	ZERO,
	V0, V1,
	A0, A1, A2, A3,
	T0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
	S0, S1, S2, S3, S4, S5, S6, S7,
	GP, SP, FP, RA
};

enum mipsOperator {
	MIPS_DATA, MIPS_TEXT,
	MIPS_SPACE, MIPS_ASCIIZ, MIPS_WORD, MIPS_WORDARR,
	MIPS_SYSCALL,
	MIPS_LABEL,
	MIPS_SW, MIPS_LW,
	MIPS_LA, MIPS_LI,
	MIPS_ADDU, MIPS_SUBU, MIPS_ADDIU, MIPS_SUBIU,
	MIPS_MUL, MIPS_DIV,
	MIPS_MFHI, MIPS_MFLO,
	MIPS_JAL, MIPS_JR, MIPS_JUMP,
	MIPS_BEQ, MIPS_BNE, MIPS_BGT, MIPS_BGE, MIPS_BLT, MIPS_BLE,
	MIPS_SLL
};

void translate();
regNo getTmpReg(std::string name, regNo defaultNo);
regNo mips_lw(std::string name, regNo defaultReg);
void mips_sw(std::string name, std::string srcReg);
