#include "Mips.h"
#include "Intercode.h"
#include "Lexical.h"
#include "Grammer.h"

std::map<std::string, Entry> localTable;
std::string tmpReg[18];
int tmpRegTime[18];

const std::string opDict[] = {
	".data", ".text",
	".space", ".asciiz", ".word", ".word",
	"syscall",
	":",
	"sw", "lw",
	"la", "li",
	"addu", "subu", "addiu", "subiu",
	"mul", "div",
	"mfhi", "mflo",
	"jal", "jr", "j",
	"beq", "bne", "bgt", "bge", "blt", "ble",
	"sll"
};

const std::string reg[] = {
	"$0",
	"$v0", "$v1",
	"$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$gp", "$sp", "$fp", "$ra"
};

bool isNumber(std::string word) {
	return (word[0] == '+' || word[0] == '-' || (word[0] >= '0' && word[0] <= '9'));
}

bool inGlobal(std::string name) {
	return globalSymbolTable.find(name) != globalSymbolTable.end();
}

bool inLocal(std::string name) {
	return localTable.find(name) != localTable.end();
}

bool isTheConst(std::string name) {
	if (inLocal(name)) return localTable[name].kind == CONST;
	else if (inGlobal(name)) return globalSymbolTable[name].kind == CONST;
	return false;
}

std::string getConst(std::string name) {
	if (inLocal(name)) return localTable[name].value;
	else return globalSymbolTable[name].value;
}

int getOffset(std::string name) {
	if (inLocal(name)) return localTable[name].offset;
	else return globalSymbolTable[name].offset;
}


void genMips(int opNo, std::string op1 = "", std::string op2 = "", std::string op3 = "") {
	switch (opNo) {
	case MIPS_DATA:
	case MIPS_SYSCALL:
		mips << opDict[opNo] << std::endl;
		break;
	case MIPS_TEXT:
		mips << std::endl << opDict[opNo] << std::endl;
		break;
	case MIPS_SPACE:
	case MIPS_WORD:
		mips << op1 << ":" << opDict[opNo] << " " << op2 << std::endl; //op1:.space op2
		break;
	case MIPS_WORDARR:
		mips << opDict[opNo] << " " << op1 << std::endl;
		break;
	case MIPS_ASCIIZ:
		mips << op1 << ":" << opDict[opNo] << " \"" << op2 << "\"" << std::endl; //op1:.asciiz "op2"
		break;
	case MIPS_JAL:
	case MIPS_JR:
	case MIPS_JUMP:
	case MIPS_MFLO:
	case MIPS_MFHI:
		mips << opDict[opNo] << " " << op1 << std::endl;
		break;
	case MIPS_LI:
	case MIPS_DIV:
		mips << opDict[opNo] << " " << op1 << ", " << op2 << std::endl;
		break;
	case MIPS_LA:
	case MIPS_LW:
	case MIPS_SW:
		mips << opDict[opNo] << " " << op1 << ", " << op2 << "(" << op3 << ")" << std::endl;
		break;
	case MIPS_LABEL:
		mips << op1 << opDict[opNo] << std::endl;
		break;
	case MIPS_ADDU:
	case MIPS_SUBU:
	case MIPS_ADDIU:
	case MIPS_SUBIU:
	case MIPS_MUL:
	case MIPS_BEQ:
	case MIPS_BNE:
	case MIPS_BGT:
	case MIPS_BGE:
	case MIPS_BLT:
	case MIPS_BLE:
	case MIPS_SLL:
		mips << opDict[opNo] << " " << op1 << ", " << op2 << "," << op3 << std::endl;
		break;
	}
}

void tmpAged() {
	for (int i = 0; i < 8; i++) {
		if (tmpReg[i] != "") tmpRegTime[i]++;
	}
	for (int i = 10; i < 18; i++) {
		if (tmpReg[i] != "") tmpRegTime[i]++;
	}
}

regNo getTmpReg(std::string name, regNo defaultNo) {
	if (name[0] != '$') return defaultNo;
	tmpAged();
	for (int i = 0; i < 8; i++) {
		if (tmpReg[i] == name) return (enum regNo)(i + 7);
	}
	for (int i = 10; i < 18; i++) {
		if (tmpReg[i] == name) return (enum regNo)(i + 7);
	}
	int max = 0;
	int place = 0;
	for (int i = 0; i < 8; i++) {
		if (tmpReg[i] == "") {
			tmpRegTime[i] = 0;
			tmpReg[i] = name;
			return (enum regNo)(i + 7);
		}
		if (tmpRegTime[i] > max) {
			place = i;
			max = tmpRegTime[i];
		}
	}
	for (int i = 10; i < 18; i++) {
		if (tmpReg[i] == "") {
			tmpRegTime[i] = 0;
			tmpReg[i] = name;
			return (enum regNo)(i + 7);
		}
		if (tmpRegTime[i] > max) {
			place = i;
			max = tmpRegTime[i];
		}
	}
	genMips(MIPS_SW, reg[(enum regNo)(place + 7)], std::to_string(localTable[tmpReg[place]].offset), reg[SP]);
	tmpRegTime[place] = 0;
	tmpReg[place] = name;
	return (enum regNo)(place + 7);
}

regNo mips_lw(std::string name, regNo defaultReg) {
	std::string dstReg = reg[defaultReg];
	if (isNumber(name)) genMips(MIPS_LI, dstReg, name);
	if (name[0] == '$') {
		if ((int)defaultReg < (int)T0 || (int)defaultReg >(int)T9) {
			for (int i = 0; i < 8; i++) {
				if (tmpReg[i] == name) {
					genMips(MIPS_ADDIU, dstReg, reg[(enum regNo)(i + 7)], "0");
					return defaultReg;
				}
			}
			for (int i = 10; i < 18; i++) {
				if (tmpReg[i] == name) {
					genMips(MIPS_ADDIU, dstReg, reg[(enum regNo)(i + 7)], "0");
					return defaultReg;
				}
			}
			genMips(MIPS_LW, dstReg, std::to_string(localTable[name].offset), reg[SP]);
			return defaultReg;
		}
		tmpAged();
		for (int i = 0; i < 8; i++) {
			if (tmpReg[i] == name) return (enum regNo)(i + 7);
		}
		for (int i = 10; i < 18; i++) {
			if (tmpReg[i] == name) return (enum regNo)(i + 7);
		}
		int max = 0;
		int place = 0;
		for (int i = 0; i < 8; i++) {
			if (tmpReg[i] == "") {
				tmpRegTime[i] = 0;
				tmpReg[i] = name;
				genMips(MIPS_LW, reg[(enum regNo)(i + 7)], std::to_string(localTable[name].offset), reg[SP]);
				return (enum regNo)(i + 7);
			}
			if (tmpRegTime[i] > max) {
				place = i;
				max = tmpRegTime[i];
			}
		}
		for (int i = 10; i < 18; i++) {
			if (tmpReg[i] == "") {
				tmpRegTime[i] = 0;
				tmpReg[i] = name;
				genMips(MIPS_LW, reg[(enum regNo)(i + 7)], std::to_string(localTable[name].offset), reg[SP]);
				return (enum regNo)(i + 7);
			}
			if (tmpRegTime[i] > max) {
				place = i;
				max = tmpRegTime[i];
			}
		}
		genMips(MIPS_SW, reg[(enum regNo)(place + 7)], std::to_string(localTable[tmpReg[place]].offset), reg[SP]);
		tmpRegTime[place] = 0;
		tmpReg[place] = name;
		genMips(MIPS_LW, reg[(enum regNo)(place + 7)], std::to_string(localTable[name].offset), reg[SP]);
		return (enum regNo)(place + 7);
	}
	if (isTheConst(name)) genMips(MIPS_LI, dstReg, getConst(name));
	else if (inLocal(name)) genMips(MIPS_LW, dstReg, std::to_string(localTable[name].offset), reg[SP]);
	else if (inGlobal(name)) genMips(MIPS_LW, dstReg, name, reg[ZERO]);
	return defaultReg;
}

void mips_sw(std::string name, std::string srcReg) {
	if (name[0] == '$') {
		tmpAged();
		for (int i = 0; i < 8; i++) {
			if (tmpReg[i] == name) {
				genMips(MIPS_ADDIU, reg[(enum regNo)(i + 7)], srcReg, "0");
				return;
			}
		}
		for (int i = 10; i < 18; i++) {
			if (tmpReg[i] == name) {
				genMips(MIPS_ADDIU, reg[(enum regNo)(i + 7)], srcReg, "0");
				return;
			}
		}
		int max = 0;
		int place = 0;
		for (int i = 0; i < 8; i++) {
			if (tmpReg[i] == "") {
				tmpRegTime[i] = 0;
				tmpReg[i] = name;
				genMips(MIPS_ADDIU, reg[(enum regNo)(i + 7)], srcReg, "0");
				return;
			}
			if (tmpRegTime[i] > max) {
				place = i;
				max = tmpRegTime[i];
			}
		}
		for (int i = 10; i < 18; i++) {
			if (tmpReg[i] == "") {
				tmpRegTime[i] = 0;
				tmpReg[i] = name;
				genMips(MIPS_ADDIU, reg[(enum regNo)(i + 7)], srcReg, "0");
				return;
			}
			if (tmpRegTime[i] > max) {
				place = i;
				max = tmpRegTime[i];
			}
		}
		genMips(MIPS_SW, reg[(enum regNo)(place + 7)], std::to_string(localTable[tmpReg[place]].offset), reg[SP]);
		tmpRegTime[place] = 0;
		tmpReg[place] = name;
		genMips(MIPS_ADDIU, reg[(enum regNo)(place + 7)], srcReg, "0");
	}
	if (inLocal(name)) genMips(MIPS_SW, srcReg, std::to_string(localTable[name].offset), reg[SP]);
	else if (inGlobal(name)) genMips(MIPS_SW, srcReg, name, reg[ZERO]);
}

void clearTmpReg(int option) {
	for (int i = 0; i < 8; i++) {
		if (tmpReg[i] != "") {
			if (option == 1) {
				genMips(MIPS_SW, reg[(enum regNo)(i + 7)], std::to_string(localTable[tmpReg[i]].offset), reg[SP]);
			}
			tmpRegTime[i] = 0;
			tmpReg[i] = "";
		}
	}
	for (int i = 10; i < 18; i++) {
		if (tmpReg[i] != "") {
			if (option == 1) {
				genMips(MIPS_SW, reg[(enum regNo)(i + 7)], std::to_string(localTable[tmpReg[i]].offset), reg[SP]);
			}
			tmpRegTime[i] = 0;
			tmpReg[i] = "";
		}
	}
}

void analysisInter(InterCode code) {
	int opNo = code.opNo;
	int nowMipsNo;
	int allOffset;
	regNo tmpNo, tmpNo1, tmpNo2;
	std::string op1 = code.op1, op2 = code.op2, op3 = code.op3;
	switch (opNo) {
	case INTER_LABEL://over
		if (op1[0] != '$') {
			clearTmpReg(0);
			localTable = allLocalTable[op1];
			mips << std::endl;
		}
		genMips(MIPS_LABEL, op1);
		if (op1[0] != '$') genMips(MIPS_SW, reg[RA], "0", reg[SP]);
		break;
	case INTER_ALLOC://over
		clearTmpReg(1);
		genMips(MIPS_SUBIU, reg[SP], reg[SP], std::to_string(globalSymbolTable[op1].offset));
		break;
	case INTER_RECYCLE://over
		genMips(MIPS_ADDIU, reg[SP], reg[SP], std::to_string(globalSymbolTable[op1].offset));
		clearTmpReg(0);
		break;
	case INTER_RET://over
		genMips(MIPS_LW, reg[RA], "0", reg[SP]);
		genMips(MIPS_JR, reg[RA]);
		break;
	case INTER_ASSIGN://over
		if (isNumber(op2)) {
			tmpNo = getTmpReg(op1, T8);
			genMips(MIPS_LI, reg[tmpNo], op2);
			if (tmpNo == T8) mips_sw(op1, reg[tmpNo]);
		}
		else {
			tmpNo = mips_lw(op2, T8);
			mips_sw(op1, reg[tmpNo]);
		}
		break;
	case INTER_WRARR:
		if (isNumber(op2)) {
			tmpNo = mips_lw(op3, T8);
			if (inLocal(op1)) genMips(MIPS_SW, reg[tmpNo], std::to_string(localTable[op1].offset - string2int(op2)), reg[SP]);
			else genMips(MIPS_SW, reg[tmpNo], std::to_string(globalSymbolTable[op1].offset + string2int(op2)), reg[ZERO]);
		}
		else {
			tmpNo1 = mips_lw(op2, T9);
			if (inLocal(op1)) {
				tmpNo2 = mips_lw(std::to_string(localTable[op1].offset), T8);
				genMips(MIPS_SUBU, reg[T9], reg[tmpNo2], reg[tmpNo1]);
				genMips(MIPS_ADDU, reg[T9], reg[T9], reg[SP]);
				tmpNo = mips_lw(op3, T8);
			}
			else {
				tmpNo2 = mips_lw(std::to_string(globalSymbolTable[op1].offset), T8);
				genMips(MIPS_ADDU, reg[T9], reg[tmpNo2], reg[tmpNo1]);
				tmpNo = mips_lw(op3, T8);
			}
			genMips(MIPS_SW, reg[tmpNo], "0", reg[T9]);
		}
		break;
	case INTER_REARR:
		tmpNo = getTmpReg(op3, T8);
		if (isNumber(op2)) {
			if (inLocal(op1)) genMips(MIPS_LW, reg[tmpNo], std::to_string(localTable[op1].offset - string2int(op2)), reg[SP]);
			else genMips(MIPS_LW, reg[tmpNo], std::to_string(globalSymbolTable[op1].offset + string2int(op2)), reg[ZERO]);
		}
		else {
			tmpNo1 = mips_lw(op2, T9);
			if (inLocal(op1)) {
				tmpNo2 = mips_lw(std::to_string(localTable[op1].offset), T8);
				genMips(MIPS_SUBU, reg[T9], reg[tmpNo2], reg[tmpNo1]);
				genMips(MIPS_ADDU, reg[T9], reg[T9], reg[SP]);
			}
			else {
				tmpNo2 = mips_lw(std::to_string(globalSymbolTable[op1].offset), T8);
				genMips(MIPS_ADDU, reg[T9], reg[tmpNo2], reg[tmpNo1]);
			}
			genMips(MIPS_LW, reg[tmpNo], "0", reg[T9]);
		}
		if (op3[0] != '$') mips_sw(op3, reg[T8]);
		break;
	case INTER_SCANFI://over
	case INTER_SCANFC:
		genMips(MIPS_LI, reg[V0], opNo == INTER_SCANFI ? "5" : "12");
		genMips(MIPS_SYSCALL);
		mips_sw(op1, reg[V0]);
		break;
	case INTER_PRINTFC://over
	case INTER_PRINTFI:
		mips_lw(op1, A0);
		genMips(MIPS_LI, reg[V0], opNo == INTER_PRINTFI ? "1" : "11");
		genMips(MIPS_SYSCALL);
		break;
	case INTER_PRINTFS://over
		genMips(MIPS_LA, reg[A0], op1, reg[ZERO]);
		genMips(MIPS_LI, reg[V0], "4");
		genMips(MIPS_SYSCALL);
		break;
	case INTER_PLUS://over
		tmpNo = getTmpReg(op1, T8);
		if (isNumber(op2) && isNumber(op3)) genMips(MIPS_LI, reg[tmpNo], std::to_string(string2int(op2) + string2int(op3)));
		else if (isNumber(op2)) {
			tmpNo1 = mips_lw(op3, T8);
			genMips(MIPS_ADDIU, reg[tmpNo], reg[tmpNo1], std::to_string(string2int(op2)));
		}
		else if (isNumber(op3)) {
			tmpNo1 = mips_lw(op2, T8);
			genMips(MIPS_ADDIU, reg[tmpNo], reg[tmpNo1], std::to_string(string2int(op3)));
		}
		else {
			tmpNo1 = mips_lw(op2, T8);
			tmpNo2 = mips_lw(op3, T9);
			genMips(MIPS_ADDU, reg[tmpNo], reg[tmpNo1], reg[tmpNo2]);
		}
		if (op1[0] != '$') mips_sw(op1, reg[T8]);
		break;
	case INTER_MINU://over
		tmpNo = getTmpReg(op1, T8);
		if (isNumber(op2) && isNumber(op3)) genMips(MIPS_LI, reg[tmpNo], std::to_string(string2int(op2) - string2int(op3)));
		else if (isNumber(op3)) {
			tmpNo1 = mips_lw(op2, T8);
			genMips(MIPS_SUBIU, reg[tmpNo], reg[tmpNo1], std::to_string(string2int(op3)));
		}
		else {
			tmpNo1 = mips_lw(op2, T8);
			tmpNo2 = mips_lw(op3, T9);
			genMips(MIPS_SUBU, reg[tmpNo], reg[tmpNo1], reg[tmpNo2]);
		}
		if (op1[0] != '$') mips_sw(op1, reg[T8]);
		break;
	case INTER_MULT://over
		tmpNo = getTmpReg(op1, T8);
		if (isNumber(op2) && isNumber(op3)) genMips(MIPS_LI, reg[tmpNo], std::to_string(string2int(op2) * string2int(op3)));
		else if (isNumber(op2)) {
			tmpNo1 = mips_lw(op3, T8);
			int multNum = string2int(op2);
			int cmpNum = 1;
			bool flag = false;
			for (int i = 0; i <= 20; i++) {
				if (cmpNum == multNum) {
					genMips(MIPS_SLL, reg[tmpNo], reg[tmpNo1], std::to_string(i));
					flag = true;
					break;
				}
			}
			if (!flag) {
				tmpNo2 = mips_lw(op2, T9);
				genMips(MIPS_MUL, reg[tmpNo], reg[tmpNo1], reg[tmpNo2]);
			}
		}
		else if (isNumber(op3)) {
			tmpNo1 = mips_lw(op2, T8);
			int multNum = string2int(op3);
			int cmpNum = 1;
			bool flag = false;
			for (int i = 0; i <= 20; i++) {
				if (cmpNum == multNum) {
					genMips(MIPS_SLL, reg[tmpNo], reg[tmpNo1], std::to_string(i));
					flag = true;
					break;
				}
				cmpNum = cmpNum * 2;
			}
			if (!flag) {
				tmpNo2 = mips_lw(op3, T9);
				genMips(MIPS_MUL, reg[tmpNo], reg[tmpNo1], reg[tmpNo2]);
			}
		}
		else {
			tmpNo1 = mips_lw(op2, T8);
			tmpNo2 = mips_lw(op3, T9);
			genMips(MIPS_MUL, reg[tmpNo], reg[tmpNo1], reg[tmpNo2]);
		}
		if (op1[0] != '$') mips_sw(op1, reg[T8]);
		break;
	case INTER_DIV://over
		tmpNo = getTmpReg(op1, T8);
		if (isNumber(op2) && isNumber(op3)) genMips(MIPS_LI, reg[tmpNo], std::to_string(string2int(op2) / string2int(op3)));
		else {
			tmpNo1 = mips_lw(op2, T8);
			tmpNo2 = mips_lw(op3, T9);
			genMips(MIPS_DIV, reg[tmpNo1], reg[tmpNo2]);
			genMips(MIPS_MFLO, reg[tmpNo]);
		}
		if (op1[0] != '$') mips_sw(op1, reg[T8]);
		break;
	case INTER_BEQ://over
	case INTER_BNE:
	case INTER_BGT:
	case INTER_BGE:
	case INTER_BLT:
	case INTER_BLE:
		nowMipsNo = opNo - INTER_BEQ + MIPS_BEQ;
		tmpNo = mips_lw(op1, T8);
		if (isNumber(op2)) genMips(nowMipsNo, reg[tmpNo], op2, op3);
		else {
			tmpNo1 = mips_lw(op2, T9);
			genMips(nowMipsNo, reg[tmpNo], reg[tmpNo1], op3);
		}
		break;
	case INTER_GOTO:
	case INTER_CALL://over
		genMips(opNo == INTER_GOTO ? MIPS_JUMP : MIPS_JAL, op1);
		break;
	case INTER_PUSH://over
		tmpNo = mips_lw(op2, T8);
		genMips(MIPS_SW, reg[tmpNo], op3, reg[SP]);
		break;
	case INTER_SVALUE://over
		mips_lw(op1, V0);
		break;
	case INTER_RVALUE://over
		mips_sw(op1, reg[V0]);
		break;
	}
}

void translate() {
	for (int i = 0; i < 18; i++) {
		tmpReg[i] = "";
		tmpRegTime[i] = 0;
	}
	genMips(MIPS_DATA);
	int globalOffset = 268500992;
	for (std::map<std::string, Entry> ::iterator iter = globalSymbolTable.begin(); iter != globalSymbolTable.end(); iter++) {
		Entry now = iter->second;
		if (now.kind != ARRAY && now.kind != VAR) continue;
		int size = 0;
		if (now.dimen >= 1) {
			if (now.dimen == 2) size = now.dimenX * now.dimenY;
			else size = now.dimenX;
			iter->second.offset = globalOffset;
			if (now.value == "0") {
				genMips(MIPS_SPACE, iter->first, std::to_string(size * 4));
			}
			else {
				std::vector<std::string> readingArray = globalArrayValue[iter->first];
				genMips(MIPS_WORD, iter->first, readingArray[0]);
				for (int i = 1; i < readingArray.size(); i++) {
					genMips(MIPS_WORDARR, readingArray[i]);
				}
			}
			globalOffset += size * 4;
		}
		else {
			genMips(MIPS_WORD, iter->first, now.value);
			globalOffset += 4;
		}
	}
	for (std::map<std::string, std::string> ::iterator iter = strTable.begin(); iter != strTable.end(); iter++) {
		genMips(MIPS_ASCIIZ, iter->first, iter->second);
	}
	genMips(MIPS_TEXT);
	genMips(MIPS_SUBIU, reg[SP], reg[SP], std::to_string(globalSymbolTable["main"].offset));
	genMips(MIPS_JAL, "main");
	genMips(MIPS_LI, reg[V0], "10");
	genMips(MIPS_SYSCALL);
	for (int i = 0; i < interCodeList.size(); i++) {
		analysisInter(interCodeList[i]);
	}
}
