#include "Lexical.h"
#include "Grammer.h"
#include "Intercode.h"
#include<iomanip>

bool globalDefineOpen = true;
std::string curName, curDefFuncName;
kinds curKind;
types curType, retType;
bool hasRet;
int curDimen;
int dimenX, dimenY, arrayNo;
int curNum, tmpNum = 0, strNum = 0, labelNum = 0;
int offset = -4;
std::string placeHolder = "placeHold";
std::map<std::string, std::map<std::string, Entry>> allLocalTable;
std::map<std::string, Entry> globalSymbolTable;//name -> global(const | var | func)
std::map<std::string, Entry> localSymbolTable;//name -> local(const | var)
std::unordered_map<std::string, std::vector<Parameter>> parameterTable;//func_name -> paraList
std::map<std::string, std::string> strTable;
std::map<std::string, std::vector<std::string>> globalArrayValue;

std::string toLower(std::string name) {
	std::string result;
	result.resize(name.size());
	transform(name.begin(), name.end(), result.begin(), ::tolower);
	return result;
}

bool inGlobalTable(std::string name) {
	name = toLower(name);
	return globalSymbolTable.find(name) != globalSymbolTable.end();
}

bool inLocalTable(std::string name) {
	name = toLower(name);
	return localSymbolTable.find(name) != localSymbolTable.end();
}

bool hasDefined(std::string name) {
	name = toLower(name);
	return inGlobalTable(name) || inLocalTable(name);
}

bool isRetFunc(std::string name) {
	name = toLower(name);
	if (!inGlobalTable(name)) {
		errorHandle('c');
		return VOID;
	}
	if (globalSymbolTable[name].kind != FUNC) {
		error("this name isnot FUNC name");
	}
	return globalSymbolTable[name].type != VOID;
}

bool isConst(std::string name) {
	name = toLower(name);
	if (inLocalTable(name)) {
		return localSymbolTable[name].kind == CONST;
	}
	else if (inGlobalTable(name)) {
		return globalSymbolTable[name].kind == CONST;
	}
	else {
		errorHandle('c');
		return false;
	}
}

types getType(std::string name) {
	name = toLower(name);
	if (inLocalTable(name)) {
		return localSymbolTable[name].type;
	}
	else if (inGlobalTable(name)) {
		return globalSymbolTable[name].type;
	}
	else {
		errorHandle('c');
		return INT;
	}
}

void insertTable(std::string name, kinds kind, types type, int dimen, std::string value = "0") {
	name = toLower(name);
	if (((globalDefineOpen || kind == FUNC) && inGlobalTable(name)) ||
		(!globalDefineOpen && inLocalTable(name))) {
		errorHandle('b');
		return;
	}
	Entry entry;
	entry.kind = kind;
	entry.type = type;
	entry.dimen = dimen;
	entry.offset = offset;
	entry.dimenX = dimenX;
	entry.dimenY = dimenY;
	entry.value = value;
	if (globalDefineOpen || kind == FUNC) {
		globalSymbolTable[name] = entry;
	}
	else {
		if (kind == ARRAY) offset -= (dimenX + 1) * (dimenY + 1) * 4;
		else offset -= 4;
		localSymbolTable[name] = entry;
	}
}

std::string genTmpVar() {
	std::string nowTmp = "$tmp" + std::to_string(tmpNum++);
	insertTable(nowTmp, VAR, INT, 0);
	return nowTmp;
}

std::string genStrVar(std::string outStr) {
	std::string nowStr = "$str" + std::to_string(strNum++);
	strTable[nowStr] = outStr;
	return nowStr;
}

std::string genLabel() {
	std::string nowLabel = "$label" + std::to_string(labelNum++);
	return nowLabel;
}

int string2int(std::string num) {
	int i = 1;
	int result = 0;
	for (int j = num.size() - 1; j > 0; j--) {
		result = result + i * (num[j] - '0');
		i = i * 10;
	}
	if (num[0] == '-') result = -result;
	else if (num[0] != '+') result = result + i * (num[0] - '0');
	return result;
}

void noSymbolInteger(std::string* op) {
	if (symbol == INTCON) {
		if (op != NULL) *op = buf;
		curNum = string2int(buf);
		getSym();
	}
	else {
		error("not integer");
	}
	record.push_back("<无符号整数>");
}

void Integer(std::string* op) {//整数
	std::string op1 = "", op2;
	if (symbol == MINU || symbol == PLUS) {
		op1 = buf;
		getSym();
	}
	noSymbolInteger(&op2);
	if (op != NULL) *op = op1 + op2;
	record.push_back("<整数>");
}

types factor(std::string* op) {
	int opNo;
	std::string op1, op2, op3;
	types tempType = VOID;
	if (symbol == INTCON || symbol == PLUS || symbol == MINU) {
		tempType = INT;
		Integer(&op2);
	}
	else if (symbol == CHARCON) {
		tempType = CHAR;
		op2 = std::to_string((int)buf[0]);
		getSym();
	}
	else if (symbol == LPARENT) {
		tempType = INT;
		getSym();
		expression(&op2);
		if (symbol == RPARENT) getSym();
		else errorHandle('l');
	}
	else if (symbol == IDENFR) {
		curName = buf;
		op2 = curName;
		tempType = getType(curName);
		getSym();
		if (symbol == LPARENT) {
			funcCallStatement();
			op2 = genTmpVar();
			genInterCode(INTER_RVALUE, op2);
		}
		else if (symbol == LBRACK) {
			Entry reading;
			std::string tmp = genTmpVar();
			if (inLocalTable(curName)) reading = localSymbolTable[curName];
			else reading = globalSymbolTable[curName];
			getSym();
			expression(&op1);
			if (symbol == RBRACK) getSym();
			if (reading.dimen == 1) {
				genInterCode(INTER_MULT, tmp, op1, "4");
				op1 = tmp;
			}
			else if (symbol == LBRACK) {
				genInterCode(INTER_MULT, tmp, op1, std::to_string(reading.dimenY));
				getSym();
				expression(&op1);
				std::string tmp1 = genTmpVar();
				genInterCode(INTER_PLUS, tmp1, tmp, op1);
				op1 = genTmpVar();
				genInterCode(INTER_MULT, op1, tmp1, "4");
				if (symbol == RBRACK) getSym();
			}
			op3 = genTmpVar();
			genInterCode(INTER_REARR, op2, op1, op3);
			op2 = op3;
		}
	}
	*op = op2;
	record.push_back("<因子>");
	return tempType;
}

types item(std::string* op) {
	int opNo = -1;
	std::string op1, op2, op3;
	types tempType = VOID;
	tempType = factor(&op2);
	while (symbol == MULT || symbol == DIV) {
		opNo = symbol == MULT ? INTER_MULT : INTER_DIV;
		tempType = INT;
		getSym();
		factor(&op3);
		op1 = genTmpVar();
		genInterCode(opNo, op1, op2, op3);
		op2 = op1;
	}
	*op = op2;
	record.push_back("<项>");
	return tempType;
}

types expression(std::string* op) {
	types tempType = VOID, storeType;
	int opNo = -1;
	std::string op1, op2 = "0", op3;
	if (symbol == PLUS || symbol == MINU) {
		opNo = symbol == PLUS ? INTER_PLUS : INTER_MINU;
		tempType = INT;
		getSym();
	}
	if (opNo == INTER_MINU) {
		item(&op3);
		op1 = genTmpVar();
		genInterCode(opNo, op1, op2, op3);
		op2 = op1;
	}
	else {
		storeType = item(&op2);
		if (tempType == VOID) tempType = storeType;
	}
	while (symbol == PLUS || symbol == MINU) {
		opNo = symbol == PLUS ? INTER_PLUS : INTER_MINU;
		tempType = INT;
		getSym();
		item(&op3);
		op1 = genTmpVar();
		genInterCode(opNo, op1, op2, op3);
		op2 = op1;
	}
	*op = op2;
	record.push_back("<表达式>");
	return tempType;
}

void defineConst() {//常量定义
	std::string op3, nowName;
	if (symbol == INTTK) {
		curType = INT;
		getSym();
		if (symbol == IDENFR) {
			curName = buf;
			getSym();
		}
		else error("no idenfr");
		if (symbol == ASSIGN) getSym();
		else error("no assign");
		Integer(&op3);
		insertTable(curName, CONST, curType, 0, op3);
		while (symbol == COMMA) {
			getSym();
			if (symbol == IDENFR) {
				curName = buf;
				getSym();
			}
			else error("no idenfr");
			if (symbol == ASSIGN) getSym();
			else error("no assign");
			Integer(&op3);
			insertTable(curName, CONST, curType, 0, op3);
		}
	}
	else if (symbol == CHARTK) {
		curType = CHAR;
		getSym();
		if (symbol == IDENFR) {
			curName = buf;
			getSym();
		}
		else error("no idenfr");
		if (symbol == ASSIGN) getSym();
		else error("no assign");
		insertTable(curName, CONST, curType, 0, std::to_string((int)buf[0]));
		if (symbol == CHARCON) getSym();
		else error("not char");
		while (symbol == COMMA) {
			getSym();
			if (symbol == IDENFR) {
				curName = buf;
				getSym();
			}
			else error("no idenfr");
			if (symbol == ASSIGN) getSym();
			else error("no assign");
			insertTable(curName, CONST, curType, 0, std::to_string((int)buf[0]));
			if (symbol == CHARCON) getSym();
			else error("not char");
		}
	}
	else {
		error("cannot find the type");
	}
	record.push_back("<常量定义>");
}

void describeConst() {//常量说明
	curKind = CONST;
	defineConst();
	if (symbol == SEMICN) getSym();
	else errorHandle('k');
	while (symbol == CONSTTK) {
		getSym();
		defineConst();
		if (symbol == SEMICN) getSym();
		else errorHandle('k');
	}
	record.push_back("<常量说明>");
}

void arraySize() {
	noSymbolInteger();
	dimenX = curNum;
	if (symbol == RBRACK) {
		getSym();
	}
	else errorHandle('m');
	while (symbol == LBRACK) {
		curDimen++;
		getSym();
		noSymbolInteger();
		dimenY = curNum;
		if (symbol == RBRACK) getSym();
		else errorHandle('m');
	}
}

void defineNoInitVar() {
	insertTable(curName, curKind, curType, curDimen);
	while (symbol == COMMA) {
		curKind = VAR;
		curDimen = 0;
		getSym();
		if (symbol == IDENFR) {
			curName = buf;
			getSym();
		}
		else error("defineNoInitVar");
		if (symbol == LBRACK) {
			curKind = ARRAY;
			curDimen++;
			getSym();
			arraySize();
		}
		insertTable(curName, curKind, curType, curDimen);
	}
	record.push_back("<变量定义无初始化>");
}

void readArray(int type) {
	int dimenRightY = 0;
	std::string op3;
	if (symbol == CHARCON) {
		if (curType == INT) errorHandle('o');
		if (globalDefineOpen) globalArrayValue[curName].push_back(std::to_string((int)buf[0]));
		else genInterCode(INTER_WRARR, curName, std::to_string(arrayNo * 4), std::to_string((int)buf[0]));
		dimenRightY++;
		arrayNo++;
		getSym();
		record.push_back("<常量>");
		while (symbol == COMMA) {
			getSym();
			if (symbol == CHARCON) {
				if (globalDefineOpen) globalArrayValue[curName].push_back(std::to_string((int)buf[0]));
				else genInterCode(INTER_WRARR, curName, std::to_string(arrayNo * 4), std::to_string((int)buf[0]));
				dimenRightY++;
				arrayNo++;
				getSym();
				record.push_back("<常量>");
			}
			else error("defineInitVar0");
		}
		if (symbol == RBRACE) getSym();
		else error("defineInitVar1");
	}
	else {
		if (curType == CHAR) errorHandle('o');
		dimenRightY++;
		Integer(&op3);
		if (globalDefineOpen) globalArrayValue[curName].push_back(op3);
		else genInterCode(INTER_WRARR, curName, std::to_string(arrayNo * 4), op3);
		arrayNo++;
		record.push_back("<常量>");
		while (symbol == COMMA) {
			dimenRightY++;
			getSym();
			Integer(&op3);
			if (globalDefineOpen) globalArrayValue[curName].push_back(op3);
			else genInterCode(INTER_WRARR, curName, std::to_string(arrayNo * 4), op3);
			arrayNo++;
			record.push_back("<常量>");
		}
		if (symbol == RBRACE) getSym();
		else error("defineInitVar1");
	}
	if ((dimenRightY != dimenY && type == 2) || (dimenRightY != dimenX && type == 1)) {
		errorHandle('n');
	}
}

void defineInitVar() {
	int dimenRight = 0, dimenRightX = 0;
	std::string op3;
	if (symbol == CHARCON) {
		insertTable(curName, curKind, curType, curDimen, std::to_string((int)buf[0]));
		if (!globalDefineOpen) {
			genInterCode(INTER_ASSIGN, curName, "", std::to_string((int)buf[0]));
		}
		if (curType == INT) errorHandle('o');
		getSym();
		record.push_back("<常量>");
	}
	else if (symbol == INTCON || symbol == PLUS || symbol == MINU) {
		if (curType == CHAR) errorHandle('o');
		Integer(&op3);
		insertTable(curName, curKind, curType, curDimen, op3);
		if (!globalDefineOpen) {
			genInterCode(INTER_ASSIGN, curName, "", op3);
		}
		record.push_back("<常量>");
	}
	else if (symbol == LBRACE) {
		insertTable(curName, curKind, curType, curDimen, "1");
		dimenRight++;
		arrayNo = 0;
		getSym();
		if (globalDefineOpen) {
			std::vector<std::string> temp;
			globalArrayValue[curName] = temp;
		}
		if (symbol == INTCON || symbol == PLUS || symbol == MINU || symbol == CHARCON) readArray(curDimen);
		else if (symbol == LBRACE) {
			dimenRight++;
			dimenRightX++;
			getSym();
			if (symbol == INTCON || symbol == PLUS || symbol == MINU || symbol == CHARCON) readArray(curDimen);
			else error("defineInitVar1");
			while (symbol == COMMA) {
				getSym();
				if (symbol == LBRACE) {
					dimenRightX++;
					getSym();
				}
				else error("defineInitVar");
				if (symbol == INTCON || symbol == PLUS || symbol == MINU || symbol == CHARCON) readArray(curDimen);
				else error("defineInitVar1");
			}
			if (symbol == RBRACE) getSym();
			else error("defineInitVar1");
		}
		else error("defineInitVar2");
		if (dimenRight != curDimen || (dimenRightX != dimenX && curDimen == 2)) {
			errorHandle('n');
		}
	}
	else error("defineInitVar");
	record.push_back("<变量定义及初始化>");
}

void defineVar() {
	curDimen = 0;
	if (symbol == LBRACK) {
		curKind = ARRAY;
		curDimen++;
		getSym();
		arraySize();
	}
	if (symbol == ASSIGN) {
		getSym();
		defineInitVar();
	}
	else {
		defineNoInitVar();
	}
	record.push_back("<变量定义>");
}

void describeVar() {
	curKind = VAR;
	defineVar();
	if (symbol == SEMICN) getSym();
	else errorHandle('k');
	int pos = record.size();
	while (symbol == INTTK || symbol == CHARTK) {
		curType = symbol == INTTK ? INT : CHAR;
		getSym();
		if (symbol == MAINTK) {
			record.insert(record.begin() + pos, "<变量说明>");
			globalDefineOpen = false;
			return;
		}
		else if (symbol == IDENFR) {
			curName = buf;
			getSym();
			if (symbol == LPARENT) {
				curDefFuncName = curName;
				record.insert(record.begin() + pos, "<变量说明>");
				globalDefineOpen = false;
				return;
			}
			else {
				curKind = VAR;
				defineVar();
				if (symbol == SEMICN) getSym();
				else errorHandle('k');
				pos = record.size();
			}
		}
		else error("describeVar");
	}
	record.push_back("<变量说明>");
	globalDefineOpen = false;
}

void parameterList() {
	std::vector<Parameter> para;
	types curSymbol;
	if (symbol == INTTK || symbol == CHARTK) {
		curSymbol = symbol == INTTK ? INT : CHAR;
		getSym();
		if (symbol == IDENFR) {
			para.push_back(Parameter{ curSymbol, buf });
			curName = buf;
			getSym();
		}
		else error("parameterlist");
		insertTable(curName, VAR, curSymbol, 0);
		while (symbol == COMMA) {
			getSym();
			if (symbol == INTTK || symbol == CHARTK) {
				curSymbol = symbol == INTTK ? INT : CHAR;
				getSym();
			}
			else error("parameterlist");
			if (symbol == IDENFR) {
				curName = buf;
				para.push_back(Parameter{ curSymbol, buf });
				getSym();
			}
			else error("parameterlist");
			insertTable(curName, VAR, curSymbol, 0);
		}
	}
	parameterTable[toLower(curDefFuncName)] = para;
	record.push_back("<参数表>");
}

void valueParaListRead() {
	if (symbol != RPARENT) {
		expression(&placeHolder);
		while (symbol == COMMA) {
			getSym();
			expression(&placeHolder);
		}
	}
}

void valueParaList(std::vector<std::string>* list) {
	std::string op1;
	if (symbol != RPARENT) {
		expression(&op1);
		(*list).push_back(op1);
		while (symbol == COMMA) {
			getSym();
			expression(&op1);
			(*list).push_back(op1);
		}
	}
	record.push_back("<值参数表>");
}

void condition(std::string endLabel) {
	types conditionType;
	int opNo = -1;
	std::string op1, op2;
	conditionType = expression(&op1);
	if (conditionType != INT) errorHandle('f');
	switch (symbol) {
	case LSS:
		opNo = INTER_BGE;
		break;
	case LEQ:
		opNo = INTER_BGT;
		break;
	case GRE:
		opNo = INTER_BLE;
		break;
	case GEQ:
		opNo = INTER_BLT;
		break;
	case EQL:
		opNo = INTER_BNE;
		break;
	case NEQ:
		opNo = INTER_BEQ;
		break;
	default:
		error("condition not compare");
	}
	getSym();
	conditionType = expression(&op2);
	if (conditionType != INT) errorHandle('f');
	genInterCode(opNo, op1, op2, endLabel);
	record.push_back("<条件>");
}

void stride(std::string* op) {
	noSymbolInteger(op);
	record.push_back("<步长>");

}

void loopStatement() {
	std::string endLabel, loopLabel;
	int opNo = -1;
	std::string op1, op2 = "", op3;
	if (symbol == FORTK) {
		getSym();
		if (symbol == LPARENT) getSym();
		else error("loopStatement IF no condition");
		if (symbol == IDENFR) {
			op1 = buf;
			if (!hasDefined(buf)) {
				errorHandle('c');
			}
			getSym();
		}
		else error("loopStatement IF no initial");
		if (symbol == ASSIGN) getSym();
		else error("loopStatement IF no initial");
		expression(&op3);
		genInterCode(INTER_ASSIGN, op1, op2, op3);
		if (symbol == SEMICN) getSym();
		else errorHandle('k');
		loopLabel = genLabel();
		genInterCode(INTER_LABEL, loopLabel);
		endLabel = genLabel();
		condition(endLabel);
		if (symbol == SEMICN) getSym();
		else errorHandle('k');
		if (symbol == IDENFR) {
			op1 = buf;
			if (!hasDefined(buf)) {
				errorHandle('c');
			}
			getSym();
		}
		else error("loopStatement IF no loop");
		if (symbol == ASSIGN) getSym();
		else error("loopStatement IF no loop");
		if (symbol == IDENFR) {
			op2 = buf;
			if (!hasDefined(buf)) {
				errorHandle('c');
			}
			getSym();
		}
		else error("loopStatement IF no loop");
		if (symbol == PLUS || symbol == MINU) {
			opNo = symbol == PLUS ? INTER_PLUS : INTER_MINU;
			getSym();
		}
		else error("loopStatement IF no loop");
		stride(&op3);
		if (symbol == RPARENT) getSym();
		else errorHandle('l');
		statement();
		genInterCode(opNo, op1, op2, op3);
		genInterCode(INTER_GOTO, loopLabel);
		genInterCode(INTER_LABEL, endLabel);
	}
	else if (symbol == WHILETK) {
		getSym();
		if (symbol == LPARENT) getSym();
		else error("loopStatement no condition");
		loopLabel = genLabel();
		genInterCode(INTER_LABEL, loopLabel);
		endLabel = genLabel();
		condition(endLabel);
		if (symbol == RPARENT) getSym();
		else errorHandle('l');
		statement();
		genInterCode(INTER_GOTO, loopLabel);
		genInterCode(INTER_LABEL, endLabel);
	}
	record.push_back("<循环语句>");
}

void ifStatement() {
	if (symbol == IFTK) getSym();
	if (symbol == LPARENT) getSym();
	else error("ifStatement no condition");
	std::string elseLabel = genLabel();
	std::string endLabel = genLabel();
	condition(elseLabel);
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	statement();
	if (symbol == ELSETK) {
		genInterCode(INTER_GOTO, endLabel);
		genInterCode(INTER_LABEL, elseLabel);
		getSym();
		statement();
		genInterCode(INTER_LABEL, endLabel);
	}
	else genInterCode(INTER_LABEL, elseLabel);
	record.push_back("<条件语句>");
}

void funcCallStatement() {
	std::string callName = curName;
	std::vector<std::string> list;
	if (isRetFunc(curName)) {
		if (symbol == LPARENT) getSym();
		valueParaList(&list);
		if (symbol == RPARENT) getSym();
		else errorHandle('l');
		record.push_back("<有返回值函数调用语句>");
	}
	else {
		if (symbol == LPARENT) getSym();
		valueParaList(&list);
		if (symbol == RPARENT) getSym();
		else errorHandle('l');
		record.push_back("<无返回值函数调用语句>");
	}
	for (int i = 0; i < list.size(); i++) {
		genInterCode(INTER_PUSH, callName, list[i], std::to_string((i + 1) * (-4)));
	}
	genInterCode(INTER_ALLOC, callName);
	genInterCode(INTER_CALL, callName);
	genInterCode(INTER_RECYCLE, callName);
}

void assignStatement() {
	int opNo;
	std::string op1 = curName, op2 = "", op3;
	opNo = INTER_ASSIGN;
	if (isConst(curName)) {
		errorHandle('j');
	}
	if (symbol == ASSIGN) {
		getSym();
		expression(&op3);
	}
	else if (symbol == LBRACK) {
		opNo = INTER_WRARR;
		Entry reading;
		std::string tmp = genTmpVar();
		if (inLocalTable(op1)) reading = localSymbolTable[op1];
		else reading = globalSymbolTable[op1];
		getSym();
		expression(&op2);
		if (symbol == RBRACK) getSym();
		if (reading.dimen == 1) {
			genInterCode(INTER_MULT, tmp, op2, "4");
			op2 = tmp;
		}
		else if (symbol == LBRACK) {
			genInterCode(INTER_MULT, tmp, op2, std::to_string(reading.dimenY));
			getSym();
			expression(&op2);
			std::string tmp1 = genTmpVar();
			genInterCode(INTER_PLUS, tmp1, tmp, op2);
			op2 = genTmpVar();
			genInterCode(INTER_MULT, op2, tmp1, "4");
			if (symbol == RBRACK) getSym();
		}
		if (symbol == ASSIGN) getSym();
		else error("no assign");
		expression(&op3);
	}
	else error("assignStatement not found");
	genInterCode(opNo, op1, op2, op3);
	record.push_back("<赋值语句>");
}

void scanfStatement() {
	int opNo = -1;
	std::string op1;
	if (symbol == SCANFTK) getSym();
	if (symbol == LPARENT) getSym();
	else error("scanf no LPARENT");
	if (symbol == IDENFR) {
		if (inLocalTable(buf)) opNo = localSymbolTable[buf].type == INT ? INTER_SCANFI : INTER_SCANFC;
		else if (inGlobalTable(buf)) opNo = globalSymbolTable[buf].type == INT ? INTER_SCANFI : INTER_SCANFC;
		op1 = buf;
		if (isConst(buf)) {
			errorHandle('j');
		}
		getSym();
	}
	else error("scanf no IDENFR");
	genInterCode(opNo, op1);
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	record.push_back("<读语句>");
}

void printfStatement() {
	int opNo = -1;
	std::string op1, op2, op3;
	if (symbol == PRINTFTK) getSym();
	if (symbol == LPARENT) getSym();
	else error("scanf no LPARENT");
	if (symbol == STRCON) {
		opNo = INTER_PRINTFS;
		op1 = genStrVar(buf);
		genInterCode(opNo, op1);
		getSym();
		record.push_back("<字符串>");
		if (symbol == COMMA) {
			getSym();
			int nowType = expression(&op1);
			opNo = nowType == INT ? INTER_PRINTFI : INTER_PRINTFC;
			genInterCode(opNo, op1);
		}
		else if (symbol == RPARENT);
		else {
			errorHandle('l');
			record.push_back("<写语句>");
			return;
		}
	}
	else {
		int nowType = expression(&op1);
		opNo = nowType == INT ? INTER_PRINTFI : INTER_PRINTFC;
		genInterCode(opNo, op1);
	}
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	genInterCode(INTER_PRINTFC, std::to_string((int)'\n'));
	record.push_back("<写语句>");
}

void defaultStatement(std::string endLabel) {
	if (symbol == DEFAULTTK) getSym();
	else {
		errorHandleInLexical('p');
		return;
	}
	if (symbol == COLON) getSym();
	else error("switchSonStatement case not");
	statement();
	record.push_back("<缺省>");
}

void switchSonStatement(types tempType, std::string endLabel, std::string compared) {
	std::string op2;
	if (symbol == CASETK) getSym();
	else error("switchSonStatement no case");
	if (symbol == CHARCON) {
		op2 = std::to_string((int)buf[0]);
		if (tempType == INT) errorHandle('o');
		getSym();
		record.push_back("<常量>");
	}
	else if (symbol == INTCON || symbol == PLUS || symbol == MINU) {
		if (tempType == CHAR) errorHandle('o');
		Integer(&op2);
		record.push_back("<常量>");
	}
	else error("no 常量");
	if (symbol == COLON) getSym();
	else error("switchSonStatement case not");
	std::string nextBranch = genLabel();
	genInterCode(INTER_BNE, compared, op2, nextBranch);
	statement();
	genInterCode(INTER_GOTO, endLabel);
	genInterCode(INTER_LABEL, nextBranch);
	record.push_back("<情况子语句>");
}

void switchList(types tempType, std::string endLabel, std::string compared) {
	switchSonStatement(tempType, endLabel, compared);
	while (symbol == CASETK) switchSonStatement(tempType, endLabel, compared);
	record.push_back("<情况表>");
}

void switchStatement() {
	types tempType = VOID;
	if (symbol == SWITCHTK) getSym();
	if (symbol == LPARENT) getSym();
	else error("switch no expression");
	std::string compared;
	tempType = expression(&compared);
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	if (symbol == LBRACE) getSym();
	else error("switch no situation");
	std::string endLabel = genLabel();
	switchList(tempType, endLabel, compared);
	defaultStatement(endLabel);
	genInterCode(INTER_LABEL, endLabel);
	if (symbol == RBRACE) getSym();
	else error("switch no situation");
	record.push_back("<情况语句>");
}

void returnStatement() {
	std::string op1;
	if (symbol == RETURNTK) getSym();
	if (symbol == LPARENT) {
		getSym();
		if (symbol == RPARENT) {
			getSym();
			if (retType == VOID) {
				errorHandle('g');
			}
			else {
				errorHandle('h');
			}
			hasRet = true;
			return;
		}
		if (retType != expression(&op1)) {
			if (retType == VOID) {
				errorHandle('g');
			}
			else {
				errorHandle('h');
			}
		}
		if (symbol == RPARENT) getSym();
		else errorHandle('l');
		genInterCode(INTER_SVALUE, op1);
	}
	else {
		if (retType == CHAR || retType == INT) errorHandle('h');
	}
	genInterCode(INTER_RET);
	hasRet = true;
	record.push_back("<返回语句>");
}

void statement() {
	if (symbol == FORTK || symbol == WHILETK) loopStatement();
	else if (symbol == IFTK)  ifStatement();
	else if (symbol == IDENFR) {
		curName = buf;
		getSym();
		if (symbol == LPARENT) {
			funcCallStatement();
			if (symbol == SEMICN) getSym();
			else errorHandle('k');
		}
		else {
			assignStatement();
			if (symbol == SEMICN) getSym();
			else errorHandle('k');
		}
	}
	else if (symbol == SCANFTK) {
		scanfStatement();
		if (symbol == SEMICN) getSym();
		else errorHandle('k');
	}
	else if (symbol == PRINTFTK) {
		printfStatement();
		if (symbol == SEMICN) getSym();
		else errorHandle('k');
	}
	else if (symbol == SWITCHTK) switchStatement();
	else if (symbol == SEMICN) getSym();
	else if (symbol == RETURNTK) {
		returnStatement();
		if (symbol == SEMICN) getSym();
		else errorHandle('k');
	}
	else if (symbol == LBRACE) {
		getSym();
		statementlist();
		if (symbol == RBRACE) getSym();
		else error("statement lost RBRACE");
	}
	else error("no such statement");
	record.push_back("<语句>");
}

void statementlist() {
	while (symbol != RBRACE) {
		statement();
	}
	record.push_back("<语句列>");
}

void compoundSen() {//复合语句
	if (symbol == CONSTTK) {
		getSym();
		describeConst();
	}
	if (symbol == INTTK || symbol == CHARTK) {
		curType = symbol == INTTK ? INT : CHAR;
		getSym();
		if (symbol == IDENFR) {
			curName = buf;
			getSym();
		}
		else error("compoundSen");
		describeVar();
	}
	statementlist();
	record.push_back("<复合语句>");
}

void defineNoRetFunc() {//无返回值函数定义
	globalDefineOpen = false;
	insertTable(curDefFuncName, FUNC, curType, 0);
	genInterCode(INTER_LABEL, curDefFuncName);
	retType = curType;
	if (symbol == LPARENT) getSym();
	else error("");
	parameterList();
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	if (symbol == LBRACE) getSym();
	else error("no sentence");
	compoundSen();
	if (symbol == RBRACE) getSym();
	else error("no sentence end");
	//printLocal();
	genInterCode(INTER_RET);
	allLocalTable[curDefFuncName] = localSymbolTable;
	globalSymbolTable[curDefFuncName].offset = -offset;
	offset = -4;
	localSymbolTable.clear();
	record.push_back("<无返回值函数定义>");
}

void defineRetFunc() {
	globalDefineOpen = false;
	insertTable(curDefFuncName, FUNC, curType, 0);
	genInterCode(INTER_LABEL, curDefFuncName);
	retType = curType;
	hasRet = false;
	parameterList();
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	if (symbol == LBRACE) getSym();
	else error("no sentence");
	compoundSen();
	if (symbol == RBRACE) getSym();
	else error("no sentence end");
	if (!hasRet) {
		errorHandle('h');
	}
	//printLocal();
	genInterCode(INTER_RET);
	allLocalTable[curDefFuncName] = localSymbolTable;
	globalSymbolTable[curDefFuncName].offset = -offset;
	offset = -4;
	localSymbolTable.clear();
	record.push_back("<有返回值函数定义>");
}

void mainFunc() {
	globalDefineOpen = false;
	retType = VOID;
	curDefFuncName = "main";
	insertTable(curDefFuncName, FUNC, VOID, 0);
	genInterCode(INTER_LABEL, curDefFuncName);
	if (symbol == LPARENT) getSym();
	else error("");
	if (symbol == RPARENT) getSym();
	else errorHandle('l');
	if (symbol == LBRACE) getSym();
	else error("no sentence");
	compoundSen();
	if (symbol == RBRACE) getSym();
	else error("no sentence end");
	genInterCode(INTER_RET);
	allLocalTable[curDefFuncName] = localSymbolTable;
	globalSymbolTable[curDefFuncName].offset = -offset;
	localSymbolTable.clear();
	record.push_back("<主函数>");
	//printLocal();
}

void grammer() {
	if (symbol == CONSTTK) {
		getSym();
		describeConst();
	}
	if (symbol == INTTK || symbol == CHARTK || symbol == VOIDTK) {
		while (symbol == INTTK || symbol == CHARTK || symbol == VOIDTK || symbol == LPARENT) {
			if (symbol == VOIDTK) {
				curType = VOID;
				getSym();
				if (symbol == IDENFR) {
					curDefFuncName = buf;
					getSym();
					defineNoRetFunc();
				}
				else if (symbol == MAINTK) {
					break;
				}
				else error("shouldnot be void");
			}
			else if (symbol == CHARTK || symbol == INTTK) {
				curType = symbol == INTTK ? INT : CHAR;
				getSym();
				if (symbol == IDENFR) {
					curName = buf;
					curDefFuncName = buf;
					getSym();
				}
				else error("no idenfr");
				if (symbol == LPARENT) {
					record.push_back("<声明头部>");
					getSym();
					defineRetFunc();
				}//非括号即变量定义（错误处理需要更改）
				else if (globalDefineOpen) {
					describeVar();
				}
				else error("describeVar second times");
			}
			else {
				record.push_back("<声明头部>");
				getSym();
				defineRetFunc();
			}
		}
	}
	else error("no type declaration");
	if (symbol == MAINTK) {
		getSym();
		mainFunc();
	}
	else error("No main");
	record.push_back("<程序>");
	optimizeLocalTable();
	printTable();
}

std::string kind2str(kinds kind) {
	switch (kind) {
	case CONST:return "CONST";
	case VAR:return "VAR";
	case ARRAY:return "ARRAY";
	case FUNC:return "FUNC";
	default:return "NOT FOUND";
	}
}

std::string type2str(types type) {
	switch (type) {
	case INT:return "INT";
	case CHAR:return "CHAR";
	case VOID:return "VOID";
	default:return "NOT FOUND";
	}
}

void printTable() {
	std::map<std::string, Entry> ::iterator iter;
	iter = globalSymbolTable.begin();
	std::cout << "GlobalTable:" << std::endl;
	for (; iter != globalSymbolTable.end(); iter++) {
		std::cout << std::setw(10) << iter->first << std::setw(10) << kind2str(iter->second.kind)
			<< std::setw(10) << type2str(iter->second.type) << std::setw(2) << iter->second.dimen << std::endl;
	}
	printLocal();
}

void printLocal() {
	std::map<std::string, std::map<std::string, Entry>> ::iterator iter;
	iter = allLocalTable.begin();
	std::cout << "LocalTable:" << std::endl;
	for (; iter != allLocalTable.end(); iter++) {
		std::map<std::string, Entry> ::iterator iter1;
		iter1 = iter->second.begin();
		std::cout << iter->first << std::endl;
		for (; iter1 != iter->second.end(); iter1++)
			std::cout << std::setw(10) << iter1->first << std::setw(10) << kind2str(iter1->second.kind)
			<< std::setw(10) << type2str(iter1->second.type) << std::setw(2) << iter1->second.dimen << "offset:" << iter1->second.offset << std::endl;
	}
}

void printPara() {
	std::unordered_map<std::string, std::vector<Parameter>> ::iterator iter;
	iter = parameterTable.begin();
	std::cout << "ParaTable:" << std::endl;
	for (; iter != parameterTable.end(); iter++) {
		std::cout << std::setw(10) << iter->first << ":" << std::endl;
		for (int i = 0; i < iter->second.size(); i++) {
			std::cout << std::setw(10) << iter->second[i].name << std::setw(10) << type2str(iter->second[i].type) << std::endl;
		}
	}
}

void optimizeLocalTable() {
	for (std::map<std::string, std::map<std::string, Entry>> ::iterator iterOut = allLocalTable.begin(); iterOut != allLocalTable.end(); iterOut++) {
		std::map<std::string, Entry> nowTable = iterOut->second;
		int totalOffset = globalSymbolTable[iterOut->first].offset;
		for (std::map<std::string, Entry> ::iterator iter = iterOut->second.begin(); iter != iterOut->second.end(); iter++) {
			iter->second.offset = totalOffset + iter->second.offset;
		}
	}
}
