#include "Lexical.h"

std::ifstream filein;
std::ofstream fileout;
std::ofstream mips;
std::string buf, prebuf;
char read;
int line = 1, preline;
Type_set symbol, presymbol;
const bool print = true;
std::vector<std::string> record;

std::unordered_map<std::string, Type_set> reservedWord = {
	{"const", CONSTTK},
	{"int", INTTK},
	{"char", CHARTK},
	{"void", VOIDTK},
	{"main", MAINTK},
	{"if", IFTK},
	{"else", ELSETK},
	{"switch",SWITCHTK},
	{"case", CASETK},
	{"default", DEFAULTTK},
	{"while", WHILETK},
	{"for", FORTK},
	{"scanf", SCANFTK},
	{"printf", PRINTFTK},
	{"return", RETURNTK},
};

std::unordered_map<char, Type_set> singleSymbol = {
	{'+', PLUS},
	{'-', MINU},
	{'*', MULT},
	{'/', DIV},
	{':', COLON},
	{';', SEMICN},
	{',', COMMA},
	{'(', LPARENT},
	{')', RPARENT},
	{'[', LBRACK},
	{']', RBRACK},
	{'{', LBRACE},
	{'}', RBRACE},
};

std::string enum2str(Type_set type) {
	switch (type) {
	case IDENFR: return "IDENFR";
	case INTCON: return "INTCON";
	case CHARCON: return "CHARCON";
	case STRCON: return "STRCON";
	case CONSTTK: return "CONSTTK";
	case INTTK: return "INTTK";
	case CHARTK: return "CHARTK";
	case VOIDTK: return "VOIDTK";
	case MAINTK: return "MAINTK";
	case IFTK: return "IFTK";
	case ELSETK: return "ELSETK";
	case SWITCHTK: return "SWITCHTK";
	case CASETK: return "CASETK";
	case DEFAULTTK: return "DEFAULTTK";
	case WHILETK: return "WHILETK";
	case FORTK: return "FORTK";
	case SCANFTK: return "SCANFTK";
	case PRINTFTK: return "PRINTFTK";
	case RETURNTK: return "RETURNTK";
	case PLUS: return "PLUS";
	case MINU: return "MINU";
	case MULT: return "MULT";
	case DIV: return "DIV";
	case LSS: return "LSS";
	case LEQ: return "LEQ";
	case GRE: return "GRE";
	case GEQ: return "GEQ";
	case EQL: return "EQL";
	case NEQ: return "NEQ";
	case COLON: return "COLON";
	case ASSIGN: return "ASSIGN";
	case SEMICN: return "SEMICN";
	case COMMA: return "COMMA";
	case LPARENT: return "LPARENT";
	case RPARENT: return "RPARENT";
	case LBRACK: return "LBRACK";
	case RBRACK: return "RBRACK";
	case LBRACE: return "LBRACE";
	case RBRACE: return "RBRACE";
	default: return "NOT FOUND";
	}
}

void open() {
	filein.open("testfile.txt");
	//fileout.open("error.txt");
	mips.open("mips.txt");
}

void close() {
	filein.close();
	//fileout.close();
	mips.close();
}

bool isNewLine() {
	return read == '\n';
}

void clearBuf() {
	buf.clear();
}

void getChar() {
	read = filein.get();
	if (isNewLine()) {
		line++;
	}
}

void addBuf() {
	buf = buf + read;
}

void retract() {
	filein.seekg(-1, std::ios_base::cur);
	if (isNewLine()) {
		line--;
	}
}

bool isSpace() {
	return read == ' ';
}


bool isTab() {
	return (read == '\t' || read == '\r');
}

bool isLetter() {
	return ((read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z') || read == '_');
}

bool isDigit() {
	return (read >= '0' && read <= '9');
}

bool isReserved() {
	std::string temp;
	temp.resize(buf.size());
	transform(buf.begin(), buf.end(), temp.begin(), ::tolower);
	if (reservedWord.find(temp) == reservedWord.end()) {
		return false;
	}
	return true;
}

bool isSingleSymbol() {
	if (singleSymbol.find(read) == singleSymbol.end()) {
		return false;
	}
	return true;
}

bool isGRE() {
	return read == '>';
}

bool isLSS() {
	return read == '<';
}

bool isEQL() {
	return read == '=';
}

bool isNOT() {
	return read == '!';
}

bool isEOF() {
	return read == EOF;
}

bool isLegalWord() {
	return ((read == 32) || (read == 33) || (read >= 35 && read <= 126));
}

void error(std::string reason) {
	std::cout << "ERROR:" << " " << reason << std::endl;
}

void errorHandle(char type) {
	//fileout << preline << " " << type << std::endl;//语法用
}

void errorHandleInLexical(char type) {
	//fileout << line << " " << type << std::endl;//词法用
}

void printPre() {
	if (print && !prebuf.empty()) {
		record.push_back(enum2str(presymbol) + " " + prebuf);
	}
}

void getSym() {
	preline = line;
	clearBuf();
	getChar();
	while (isSpace() || isNewLine() || isTab()) {
		getChar();
	}
	if (isLetter()) {
		while (isLetter() || isDigit()) {
			addBuf();
			getChar();
		}
		retract();
		if (isReserved()) {
			std::string temp;
			temp.resize(buf.size());
			transform(buf.begin(), buf.end(), temp.begin(), ::tolower);
			symbol = reservedWord[temp];
		}
		else {
			symbol = IDENFR;
		}
	}
	else if (isDigit()) {
		while (isDigit()) {
			addBuf();
			getChar();
		}
		retract();
		symbol = INTCON;
	}
	else if (isSingleSymbol()) {
		addBuf();
		symbol = singleSymbol[read];
	}
	else if (isGRE()) {
		addBuf();
		getChar();
		if (isEQL()) {
			addBuf();
			symbol = GEQ;
		}
		else {
			retract();
			symbol = GRE;
		}
	}
	else if (isLSS()) {
		addBuf();
		getChar();
		if (isEQL()) {
			addBuf();
			symbol = LEQ;
		}
		else {
			retract();
			symbol = LSS;
		}
	}
	else if (isEQL()) {
		addBuf();
		getChar();
		if (isEQL()) {
			addBuf();
			symbol = EQL;
		}
		else {
			retract();
			symbol = ASSIGN;
		}
	}
	else if (isNOT()) {
		addBuf();
		getChar();
		if (isEQL()) {
			addBuf();
			symbol = NEQ;
		}
		else {
			error("(!=)");
		}
	}
	else if (read == '\'') {
		symbol = CHARCON;
		getChar();
		if (isDigit() || isLetter() || read == '+' ||
			read == '-' || read == '*' || read == '/') {
			addBuf();
		}
		else {
			errorHandleInLexical('a');
		}
		getChar();
		if (read != '\'') {
			errorHandleInLexical('a');
		}
	}
	else if (read == '\"') {
		getChar();
		symbol = STRCON;
		if (read == '\"') errorHandleInLexical('a');
		while (isLegalWord()) {
			addBuf();
			getChar();
			if (read == '\\') buf = buf + '\\';
		}
		if (read != '\"') {
			errorHandleInLexical('a');
		}
		while (read != '\"') {
			addBuf();
			getChar();
		}
	}
	else if (isEOF()) {
		printPre();
		return;
	}
	else {
		errorHandleInLexical('a');
	}
	printPre();
	presymbol = symbol;
	prebuf = buf;
	if (symbol != CHARCON && symbol != STRCON)
		transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
}

void printRecord() {
	for (int i = 0; i < record.size(); i++) {
		fileout << record[i] << std::endl;
	}
}
