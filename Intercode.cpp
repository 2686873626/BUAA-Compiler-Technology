#include "Intercode.h"


const std::string opDict[] = {
	"=",
	"+", "-", "*", "/",
	"push", "call", "ret",
	"beq", "bne", "bgt", "bge", "blt", "ble",
	"[]=", "=[]",
	":",
	"goto",
	"printfs", "printfi", "printfc",
	"scanfi", "scanfc",
	"alloc", "recycle",
	"put", "get"
};

std::vector<InterCode> interCodeList;

void genInterCode(int opNo, std::string op1, std::string op2, std::string op3) {
	switch (opNo) {
	case INTER_ASSIGN:
		interCodeList.push_back({ opNo, op1, op3 });
		std::cout << op1 << opDict[opNo] << op3 << std::endl;
		break;
	case INTER_PLUS:
	case INTER_MINU:
	case INTER_MULT:
	case INTER_DIV:
		interCodeList.push_back({ opNo, op1, op2, op3 });
		std::cout << op1 << "=" << op2 << opDict[opNo] << op3 << std::endl;
		break;
	case INTER_WRARR:
		interCodeList.push_back({ opNo, op1, op2, op3 });
		std::cout << op1 << "[" << op2 << "]=" << op3 << std::endl;//op2 = offset
		break;
	case INTER_REARR:
		interCodeList.push_back({ opNo, op1, op2, op3 });
		std::cout << op3 << "=" << op1 << "[" << op2 << "]" << std::endl;//op2 = offset
		break;
	case INTER_SCANFI:
	case INTER_SCANFC:
	case INTER_PRINTFS:
	case INTER_PRINTFI:
	case INTER_PRINTFC:
	case INTER_GOTO:
	case INTER_CALL:
		interCodeList.push_back({ opNo, op1 });
		std::cout << opDict[opNo] << " " << op1 << std::endl;
		break;
	case INTER_LABEL:
		interCodeList.push_back({ opNo, op1 });
		std::cout << op1 << opDict[opNo] << std::endl;
		break;
	case INTER_BEQ:
	case INTER_BNE:
	case INTER_BLT:
	case INTER_BLE:
	case INTER_BGT:
	case INTER_BGE:
		interCodeList.push_back({ opNo, op1, op2, op3 });
		std::cout << opDict[opNo] << " " << op1 << " " << op2 << " " << op3 << std::endl;
		break;
	case INTER_ALLOC:
	case INTER_RECYCLE:
	case INTER_SVALUE:
	case INTER_RVALUE:
		interCodeList.push_back({ opNo, op1 });
		std::cout << opDict[opNo] << " " << op1 << std::endl;
		break;
	case INTER_PUSH:
		interCodeList.push_back({ opNo, op1, op2, op3 });
		std::cout << opDict[opNo] << " " << op1 << " " << op2 << " " << op3 << std::endl;
		break;
	case INTER_RET:
		interCodeList.push_back({ opNo });
		std::cout << opDict[opNo] << std::endl;
		break;
	}
}

void printList(int opNo, std::string op1, std::string op2, std::string op3) {
	switch (opNo) {
	case INTER_ASSIGN:
		std::cout << op1 << opDict[opNo] << op2 << std::endl;
		break;
	case INTER_PLUS:
	case INTER_MINU:
	case INTER_MULT:
	case INTER_DIV:
		std::cout << op1 << "=" << op2 << opDict[opNo] << op3 << std::endl;
		break;
	case INTER_SCANFI:
	case INTER_SCANFC:
	case INTER_PRINTFS:
	case INTER_PRINTFI:
	case INTER_PRINTFC:
		std::cout << opDict[opNo] << " " << op1 << std::endl;
		break;
	case INTER_LABEL:
		std::cout << op1 << opDict[opNo] << std::endl;
		break;
	}
}

void optimizeCode() {
	int nowSize = interCodeList.size() - 1;
	for (int i = 0; i < nowSize; i++) {
		if (interCodeList[i].op1[0] == '$' && interCodeList[i + 1].op2 == interCodeList[i].op1 && interCodeList[i + 1].opNo == INTER_ASSIGN) {
			interCodeList[i].op1 = interCodeList[i + 1].op1;
			interCodeList.erase(interCodeList.begin() + i + 1);
			nowSize--;
		}
	}
	for (int i = 0; i < interCodeList.size(); i++) {
		printList(interCodeList[i].opNo, interCodeList[i].op1, interCodeList[i].op2, interCodeList[i].op3);
	}
}
