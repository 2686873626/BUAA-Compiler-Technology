#pragma once
#include<iostream>
#include<vector>

enum interOp {
	INTER_ASSIGN,
	INTER_PLUS, INTER_MINU, INTER_MULT, INTER_DIV,
	INTER_PUSH, INTER_CALL, INTER_RET,
	INTER_BEQ, INTER_BNE, INTER_BGT, INTER_BGE, INTER_BLT, INTER_BLE,
	INTER_WRARR, INTER_REARR,
	INTER_LABEL,
	INTER_GOTO,
	INTER_PRINTFS, INTER_PRINTFI, INTER_PRINTFC,
	INTER_SCANFI, INTER_SCANFC,
	INTER_ALLOC, INTER_RECYCLE,
	INTER_SVALUE, INTER_RVALUE
};

/*const std::string opDict[] = {
	"=",
	"+", "-", "*", "/",
	"push", "call", "ret",
	"beq", "bne", "bgt", "bge", "blt", "ble",
	":",
	"goto",
	"printfs", "printfi", "printfc",
	"scanfi", "scanfc"
};*/
struct InterCode {
	int opNo;
	std::string op1, op2, op3;
};

extern std::vector<InterCode> interCodeList;

void genInterCode(int opNo, std::string op1 = "", std::string op2 = "", std::string op3 = "");
void optimizeCode();
